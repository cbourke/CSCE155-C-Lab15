#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>

#include "games.h"

SQLRETURN addVideoGame(char* name, int publisherID, SQLHANDLE handle) {

  VideoGame* g = getGame(name, handle);

  if (g != NULL) {
    fprintf(stderr, "Attempt to add game %s aborted: game with that name already exists.\n",
        name);
    return SQL_NO_DATA;
  }

  SQLCHAR query[256];

  sprintf(query, "INSERT INTO game(name, publisher_id) VALUES (?, ?)");

  // 3rd argument is the length of the query string, but passing SQL_NTS
  // flags it as a null terminated string so that we don't have to do it
  // ourselves
  SQLPrepare(handle, query, SQL_NTS);

  // bind the parameters
  // details:
  // http://msdn.microsoft.com/en-us/library/windows/desktop/ms710963(v=vs.85).aspx
  SQLLEN zero = 0;
  SQLLEN nts = SQL_NTS;
  SQLBindParameter(handle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR,
                   strlen(name), 0, name, 0, &nts);
  SQLBindParameter(handle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0,
                   &publisherID, 0, &zero);

  SQLRETURN returnVal = SQLExecute(handle);

  return returnVal;
}

SQLRETURN addPlatform(char* name, SQLHANDLE handle) {
  
  Platform* p = getPlatform(name, handle);
  
  if (p != NULL) {
    fprintf(stderr, "Attempt to add platform %s aborted: platform with that name already exists.\n", name);
    return SQL_NO_DATA;
  }

  SQLCHAR query[256];

  //an unprepared, unsanitized query
  sprintf(query, "INSERT INTO platform(name) VALUES(\"%s\")", name);

  // Sends the query to the SQL server
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);

  SQLCloseCursor(handle);

  // Returns the status of the query
  return returnVal;
}

SQLRETURN addPublisher(char* name, SQLHANDLE handle) {

  Publisher* p = getPublisher(name, handle);

  if (p != NULL) {
    fprintf(stderr, "Attempt to add publisher %s aborted: publisher with that name already exists.\n", name);
    return SQL_NO_DATA;
  }

  SQLCHAR query[256];

  sprintf(query, "INSERT INTO publisher(name) VALUES(\"%s\")", name);

  // Sends the query to the SQL server
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);
  SQLCloseCursor(handle);

  return returnVal;
}

SQLRETURN addAvailability(int gameID, int platformID, int publishYear,
                          SQLHANDLE handle) {

  SQLCHAR query[256];

  sprintf(query, "INSERT INTO availability(game_id, platform_id, publish_year) VALUES(%d, %d, %d)",
          gameID, platformID, publishYear);

  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);
  SQLCloseCursor(handle);

  return returnVal;
}

VideoGame** getAllGames(int* numGames, SQLHANDLE handle) {

  VideoGame** allGames;  // Holds each game
  char** names;  // Names of all retrieved games
  char name[1024];  // Name of a single game

  // Gets all entries in the game table from the DB
  SQLCHAR query[256];
  sprintf(query, "SELECT * FROM game");
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);
  SQLRowCount(handle, (SQLLEN*)numGames);

  // Allocates space for the pointers to the individual games
  allGames = (VideoGame**)malloc(sizeof(VideoGame*) * (*numGames));

  // We can't use the handle for getGame while retrieving the results from
  // the current SQL query, so we need to read in all the game names first,
  // then use those to retrieve the games in a separate query
  names = (char**)malloc(sizeof(char*) * (*numGames));
  for(int i=0; i < *numGames; i++) {
    names[i] = (char*)malloc(sizeof(char) * 1024);
  }

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    SQLBindCol(handle, 2, SQL_C_CHAR, name, sizeof(char) * 1024, NULL);
  } else {
    return NULL;
  }

  // Actually retrieves the fetched game name
  for(int i = 0; SQL_SUCCEEDED(returnVal = SQLFetch(handle)); i++) {
    strcpy(names[i], name);
  }

  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  SQLCloseCursor(handle);

  // For each name retrieved, create a complete VideoGame instance
  for(int i = 0; i < *numGames; i++) {
    allGames[i] = getGame(names[i], handle);
    // Releases memory allocated for each specific game
    free(names[i]);
  }

  free(names);
  return allGames;
}

void getAvailability(VideoGame* game, SQLHANDLE handle) {

  int platform_id;
  int publish_year;

  SQLCHAR query[256];
  SQLLEN numSelRows;

  Platform** platforms;

  // Gets all entries in availability for the specific game
  sprintf(query, "SELECT * FROM availability WHERE game_id = %d", game->id);
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);

  // Issue query, get number of rows returned
  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    SQLBindCol(handle, 3, SQL_INTEGER, &platform_id, sizeof(int), NULL);
    SQLBindCol(handle, 4, SQL_INTEGER, &publish_year, sizeof(int), NULL);
  } else {
    fprintf(stderr, "Could not grab availability for game %s\n", game->name);
  }
  SQLRowCount(handle, &numSelRows);

  // Store number of entries returned by the SQL query and allocate space for
  // platforms
  platforms = (Platform**)malloc(sizeof(Platform*) * numSelRows);
  game->years = (int*)malloc(sizeof(int) * numSelRows);
  game->numPlatforms = numSelRows;

  // Fills all platforms in game with year and ID (need separate select for
  // platform name)
  for (int i = 0; SQL_SUCCEEDED(returnVal = SQLFetch(handle)); i++) {
    platforms[i] = (Platform*)malloc(sizeof(Platform));
    platforms[i]->id = platform_id;
    game->years[i] = publish_year;
  }
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);
  SQLCloseCursor(handle);

  // Gets platform name
  for (int i = 0; i < numSelRows; i++) {
    // Gets current platform name from ID
    sprintf(query, "SELECT * FROM platform WHERE platform_id = %d",
            platforms[i]->id);
    SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);

    if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
      SQLBindCol(handle, 2, SQL_C_CHAR, platforms[i]->name, sizeof(char) * 1024,
                 NULL);
      SQLFetch(handle);
    } else {
      fprintf(stderr, "Error retrieving platform with id %d from DB\n",
              platforms[i]->id);
    }
    SQLCloseCursor(handle);
  }

  // Resets handle so it can be used again safely and releases memory
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);

  // Sets the entry in the passed game to the found platforms
  game->platforms = platforms;
}

VideoGame* getGame(char* name, SQLHANDLE handle) {

  VideoGame* game = malloc(sizeof(VideoGame));

  int publisherID;

  SQLCHAR query[256];

  sprintf(query, "SELECT * FROM game WHERE name = \"%s\"", name);
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    // check that there is a record
    int numResults = 0;
    SQLRowCount(handle, (SQLLEN*)&numResults);
    if (numResults == 0) {
      return NULL;
    } else if (numResults > 1) {
      fprintf(stderr, "WARNING: multiple records (%d) exist for game '%s', returning the first one...\n",
          numResults, name);
    }

    // Maps expected columns to structure/publisherID
    SQLBindCol(handle, 1, SQL_INTEGER, &game->id, sizeof(game->id),
               NULL);
    SQLBindCol(handle, 2, SQL_C_CHAR, &game->name, sizeof(char) * 1024,
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

  // Publisher's default value should be NULL for each game (incase no publisher
  // is found)
  game->publisher = NULL;

  // Get publisher name from DB given the ID found in the previous query
  char pubName[1024];
  sprintf(query, "SELECT * FROM publisher WHERE publisher_id = %d", publisherID);
  returnVal = SQLExecDirect(handle, query, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    SQLBindCol(handle, 2, SQL_C_CHAR, pubName, sizeof(char) * 1024, NULL);
    SQLFetch(handle);
  } else {
    fprintf(stderr, "Error retrieving publisher for %s from DB\n", name);
  }
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);

  // Grabs publisher
  game->publisher = getPublisher(pubName, handle);

  // Fills out rest of the game (year, platform)
  getAvailability(game, handle);

  return game;
}

Platform* getPlatform(char* name, SQLHANDLE handle) {

  Platform* platform = malloc(sizeof(Platform));

  SQLCHAR query[256];

  sprintf(query, "SELECT * FROM platform WHERE name = \"%s\"", name);
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);

  if (returnVal == SQL_SUCCESS || returnVal == SQL_SUCCESS_WITH_INFO) {
    // check that there is a record
    int numResults = 0;
    SQLRowCount(handle, (SQLLEN*)&numResults);
    if (numResults == 0) {
      return NULL;
    } else if (numResults > 1) {
      fprintf(stderr, "WARNING: multiple records (%d) exist for platform '%s', returning the first one...\n",
          numResults, name);
    }

    // Maps expected columns to structure/publisherID
    SQLBindCol(handle, 1, SQL_INTEGER, &platform->id,
               sizeof(platform->id), NULL);
    SQLBindCol(handle, 2, SQL_C_CHAR, &platform->name, sizeof(char) * 1024,
               NULL);

    // Gets data from DB and pushes into bound variables above
    returnVal = SQLFetch(handle);
  } else {
    return NULL;
  }

  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_UNBIND);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);

  return platform;
}

Publisher* getPublisher(char* name, SQLHANDLE handle) {

  Publisher* publisher = malloc(sizeof(Publisher));

  SQLCHAR query[256];

  // Selects each entry in the publisher table matching the passed name
  sprintf(query, "SELECT * FROM publisher WHERE name = \"%s\"", name);
  SQLRETURN returnVal = SQLExecDirect(handle, query, SQL_NTS);

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
    SQLBindCol(handle, 1, SQL_INTEGER, &publisher->id,
               sizeof(publisher->id), NULL);
    SQLBindCol(handle, 2, SQL_C_CHAR, &publisher->name, sizeof(char) * 1024,
               NULL);

    // Gets data from DB and pushes into bound variables above
    returnVal = SQLFetch(handle);
  } else {
    return NULL;
  }

  SQLFreeStmt(handle, SQL_UNBIND);
  SQLCloseCursor(handle);
  SQLFreeStmt(handle, SQL_RESET_PARAMS);

  return publisher;
}
