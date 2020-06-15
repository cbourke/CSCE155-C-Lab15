#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>

#include "games.h"

// Inserts game into database.  Returns SQLExecDirect return value.
SQLRETURN addVideoGame(char* name, int publisherID, SQLHANDLE handle) {
  VideoGame* g = getGame(name, handle);
  if (g != NULL) {
    printf(
        "Attempt to add game %s aborted: game with that name already exists.\n",
        name);
    return SQL_NO_DATA;
  }

  // Calculates needed string length for the command
  SQLRETURN returnVal;
  SQLCHAR* command;

  // Send game info to DB
  //    asprintf(&command, "INSERT INTO game(name, publisher_id)"
  //         " VALUES(\"%s\", %d)", name, publisherID);

  asprintf(&command, "INSERT INTO game(name, publisher_id) VALUES (?, ?)", name,
           publisherID);

  // 3rd argument is the length of the query string, but passing SQL_NTS
  // flags it as a null terminated string so that we don't have to do it
  // ourselves
  SQLPrepare(handle, command, SQL_NTS);

  // bind the parameters
  // details:
  // http://msdn.microsoft.com/en-us/library/windows/desktop/ms710963(v=vs.85).aspx
  SQLLEN zero = 0;
  SQLLEN nts = SQL_NTS;
  SQLBindParameter(handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                   strlen(name), 0, name, 0, &nts);
  SQLBindParameter(handle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
                   &publisherID, 0, &zero);

  returnVal = SQLExecute(handle);

  //    extractError("", handle, SQL_HANDLE_STMT);

  //    returnVal = SQLExecDirect(handle, command, SQL_NTS);
  // SQLCloseCursor(handle);
  free(command);

  return returnVal;
}

// Adds a platform with the given name to the DB
SQLRETURN addPlatform(char* name, SQLHANDLE handle) {
  Platform* p = getPlatform(name, handle);
  if (p != NULL) {
    printf(
        "Attempt to add platform %s aborted: platform with that name already "
        "exists.\n",
        name);
    return SQL_NO_DATA;
  }

  // Calculates needed string length for the command
  SQLRETURN returnVal;
  SQLCHAR* command;

  // Command to send to the DB
  asprintf(&command, "INSERT INTO platform(name) VALUES(\"%s\")", name);

  // Sends the command to the SQL server
  returnVal = SQLExecDirect(handle, command, SQL_NTS);

  // Command needs to be freed since asprintf allocates it on the heap
  free(command);
  SQLCloseCursor(handle);

  // Returns the status of the command
  return returnVal;
}

// Adds a publisher with the given name to the DB
SQLRETURN addPublisher(char* name, SQLHANDLE handle) {
  Publisher* p = getPublisher(name, handle);
  if (p != NULL) {
    printf(
        "Attempt to add publisher %s aborted: publisher with that name already "
        "exists.\n",
        name);
    return SQL_NO_DATA;
  }

  // Calculates needed string length for the command
  SQLRETURN returnVal;
  SQLCHAR* command;

  asprintf(&command, "INSERT INTO publisher(name) VALUES(\"%s\")", name);

  // Sends the command to the SQL server
  returnVal = SQLExecDirect(handle, command, SQL_NTS);
  free(command);
  SQLCloseCursor(handle);

  return returnVal;
}

// Adds a relationship between game, platform, and year published to the DB
SQLRETURN addAvailability(int gameID, int platformID, int publishYear,
                          SQLHANDLE handle) {
  // Calculates needed string length for the command
  SQLRETURN returnVal;
  SQLCHAR* command;

  // Send game info to DB
  asprintf(&command,
           "INSERT INTO availability(game_id, platform_id,"
           "publish_year) VALUES(%d, %d, %d)",
           gameID, platformID, publishYear);

  returnVal = SQLExecDirect(handle, command, SQL_NTS);
  SQLCloseCursor(handle);
  free(command);

  return returnVal;
}

// Grabs every game in the database and returns an array of pointers to each
VideoGame** getAllGames(int* numGames, SQLHANDLE handle) {
  SQLRETURN returnVal;
  SQLCHAR* command;

  VideoGame** allGames;  // Holds each game
  char** names;  // Names of all retrieved games
  char name[1024];  // Name of a single game
  int i;  // Generic loop counter

  // Gets all entries in the game table from the DB
  asprintf(&command, "SELECT * FROM game");
  returnVal = SQLExecDirect(handle, command, SQL_NTS);
  SQLRowCount(handle, (SQLLEN*)numGames);
  free(command);

  // Allocates space for the pointers to the individual games
  allGames = (VideoGame**)malloc(sizeof(VideoGame*) * (*numGames));

  // We can't use the handle for getGame while retrieving the results from
  // the current SQL command, so we need to read in all the game names first,
  // then use those to retrieve the games in a separate command
  names = (char**)malloc(sizeof(char*) * (*numGames));
  for (i = 0; i < *numGames; i++) names[i] = (char*)malloc(sizeof(char) * 1024);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    SQLBindCol(handle, 2, SQL_C_CHAR, name, sizeof(char) * 1024, NULL);
  } else {
    return NULL;
  }

  // Actually retrieves the fetched game name
  for (i = 0; SQL_SUCCEEDED(returnVal = SQLFetch(handle)); i++) {
    strcpy(names[i], name);
  }

  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  SQLCloseCursor(handle);

  // For each name retrieved, create a complete VideoGame type
  for (i = 0; i < *numGames; i++) {
    allGames[i] = getGame(names[i], handle);
    // Releases memory allocated for each specific game
    free(names[i]);
  }

  free(names);
  return allGames;
}

// Gets all platforms for a game, as well as the
// published years(malloc'ed) for each platform for the game
void getAvailability(VideoGame* game, SQLHANDLE handle) {
  int platform_id;
  int publish_year;
  int i;

  SQLRETURN returnVal;
  SQLCHAR* command;
  SQLLEN numSelRows;

  Platform** platforms;

  // Gets all entries in availability for the specific game
  asprintf(&command, "SELECT * FROM availability WHERE game_id = %d", game->id);
  returnVal = SQLExecDirect(handle, command, SQL_NTS);

  // Issue query, get number of rows returned
  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    SQLBindCol(handle, 3, SQL_INTEGER, &platform_id, sizeof(int), NULL);
    SQLBindCol(handle, 4, SQL_INTEGER, &publish_year, sizeof(int), NULL);
  } else {
    fprintf(stderr, "Could not grab availability for game %s\n", game->name);
  }
  SQLRowCount(handle, &numSelRows);
  free(command);

  // Store number of entries returned by the SQL command and allocate space for
  // platforms
  platforms = (Platform**)malloc(sizeof(Platform*) * numSelRows);
  game->years = (int*)malloc(sizeof(int) * numSelRows);
  game->numPlatforms = numSelRows;

  // Fills all platforms in game with year and ID (need separate select for
  // platform name)
  for (i = 0; SQL_SUCCEEDED(returnVal = SQLFetch(handle)); i++) {
    platforms[i] = (Platform*)malloc(sizeof(Platform));
    platforms[i]->id = platform_id;
    game->years[i] = publish_year;
  }
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  SQLCloseCursor(handle);

  // Gets platform name
  for (i = 0; i < numSelRows; i++) {
    // Gets current platform name from ID
    asprintf(&command, "SELECT * FROM platform WHERE platform_id = %d",
             platforms[i]->id);
    returnVal = SQLExecDirect(handle, command, SQL_NTS);

    // Success?
    if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
      SQLBindCol(handle, 2, SQL_C_CHAR, platforms[i]->name, sizeof(char) * 1024,
                 NULL);
      SQLFetch(handle);
    } else {
      fprintf(stderr, "Error retrieving platform with id %d from DB\n",
              platforms[i]->id);
    }
    SQLCloseCursor(handle);
    free(command);
  }

  // Resets handle so it can be used again safely and releases memory
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);

  // Sets the entry in the passed game to the found platforms
  game->platforms = platforms;
}

// Gets a single game from the database
VideoGame* getGame(char* name, SQLHANDLE handle) {
  VideoGame* thisGame = malloc(sizeof(VideoGame));

  int platformID;
  int publisherID;

  SQLRETURN returnVal;
  SQLCHAR* command;

  // Grabs all games from the 'game' table matching the passed name
  asprintf(&command, "SELECT * FROM game WHERE name = \"%s\"", name);
  returnVal = SQLExecDirect(handle, command, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    // check that there is a record
    int numResults = 0;
    SQLRowCount(handle, (SQLLEN*)&numResults);
    if (numResults == 0) {
      return NULL;
    } else if (numResults > 1) {
      printf(
          "WARNING: multiple records (%d) exist for game '%s', returning the "
          "first one...\n",
          numResults, name);
    }

    // Maps expected columns to structure/publisherID
    SQLBindCol(handle, 1, SQL_INTEGER, &thisGame->id, sizeof(thisGame->id),
               NULL);
    SQLBindCol(handle, 2, SQL_C_CHAR, &thisGame->name, sizeof(char) * 1024,
               NULL);
    SQLBindCol(handle, 3, SQL_INTEGER, &publisherID, sizeof(int), NULL);

    // Gets data from DB and pushes into bound variables above
    returnVal = SQLFetch(handle);
  } else {
    return NULL;
  }

  // Safely releases handle for reuse
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  free(command);

  // Publisher's default value should be NULL for each game (incase no publisher
  // is found)
  thisGame->publisher = NULL;

  // Get publisher name from DB given the ID found in the previous query
  char pubName[1024];
  asprintf(&command, "SELECT * FROM publisher WHERE publisher_id = %d",
           publisherID);
  returnVal = SQLExecDirect(handle, command, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    SQLBindCol(handle, 2, SQL_C_CHAR, pubName, sizeof(char) * 1024, NULL);
    SQLFetch(handle);
  } else {
    fprintf(stderr, "Error retrieving publisher for %s from DB\n", name);
  }
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  free(command);

  // Grabs publisher
  thisGame->publisher = getPublisher(pubName, handle);

  // Fills out rest of thisGame (year, platform)
  getAvailability(thisGame, handle);

  return thisGame;
}

// Gets a single platform struct from the DB
Platform* getPlatform(char* name, SQLHANDLE handle) {
  Platform* thisPlatform = malloc(sizeof(Platform));

  SQLRETURN returnVal;
  SQLCHAR* command;

  asprintf(&command, "SELECT * FROM platform WHERE name = \"%s\"", name);
  returnVal = SQLExecDirect(handle, command, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    // check that there is a record
    int numResults = 0;
    SQLRowCount(handle, (SQLLEN*)&numResults);
    if (numResults == 0) {
      return NULL;
    } else if (numResults > 1) {
      printf(
          "WARNING: multiple records (%d) exist for platform '%s', returning "
          "the first one...\n",
          numResults, name);
    }

    // Maps expected columns to structure/publisherID
    SQLBindCol(handle, 1, SQL_INTEGER, &thisPlatform->id,
               sizeof(thisPlatform->id), NULL);
    SQLBindCol(handle, 2, SQL_C_CHAR, &thisPlatform->name, sizeof(char) * 1024,
               NULL);

    // Gets data from DB and pushes into bound variables above
    returnVal = SQLFetch(handle);
  } else {
    return NULL;
  }

  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  free(command);

  return thisPlatform;
}

// Takes name of publisher, handle, returns publisher struct
Publisher* getPublisher(char* name, SQLHANDLE handle) {
  Publisher* thisPublisher = malloc(sizeof(Publisher));

  SQLRETURN returnVal;
  SQLCHAR* command;

  // Selects each entry in the publisher table matching the passed name
  asprintf(&command, "SELECT * FROM publisher WHERE name = \"%s\"", name);
  returnVal = SQLExecDirect(handle, command, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    // check that there is a record
    int numResults = 0;
    SQLRowCount(handle, (SQLLEN*)&numResults);
    if (numResults == 0) {
      return NULL;
    } else if (numResults > 1) {
      printf(
          "WARNING: multiple records (%d) exist for publisher '%s', returning "
          "the first one...\n",
          numResults, name);
    }

    // Maps expected columns to structure/publisherID
    SQLBindCol(handle, 1, SQL_INTEGER, &thisPublisher->id,
               sizeof(thisPublisher->id), NULL);
    SQLBindCol(handle, 2, SQL_C_CHAR, &thisPublisher->name, sizeof(char) * 1024,
               NULL);

    // Gets data from DB and pushes into bound variables above
    returnVal = SQLFetch(handle);
  } else {
    return NULL;
  }

  SQLFreeStmt(handle, SQL_UNBIND);
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  free(command);

  return thisPublisher;
}
