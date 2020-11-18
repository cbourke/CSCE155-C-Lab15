/**
 * Uses ODBC to establish a connection to a mySQL database
 * filled with various game information.
 */
#include <stdlib.h>
#include <stdio.h>
#include <sqltypes.h>
#include <sql.h>
#include <sqlext.h>

#include "games.h"
#include "odbc_utils.h"
#include "databaseInfo.h"

int main(int argc, char** argv) {
  int i;

  // Each stores various information about the connection
  SQLHANDLE sqlenvhandle;
  SQLHANDLE sqlconnectionhandle;
  SQLHANDLE sqlstatementhandle;

  // String to store info that will be used to connect
  char connectionInfo[1000];

  // Create the string containing information about the database connection
  sprintf(connectionInfo,
          "DRIVER={%s};SERVER=%s;"
          "PORT=%s;DATABASE=%s;UID=%s;PWD=%s;",
          driver, server, port, database, userID, password);

  // Connect to the DB.  Exit if there's a problem.
  if (!setupConnection(&sqlenvhandle, &sqlconnectionhandle, &sqlstatementhandle,
                       connectionInfo)) {
    // Cleans up the connection before exiting
    finishConnection(sqlenvhandle, sqlstatementhandle, sqlconnectionhandle);
    return -1;
  }

  int numGames;
  VideoGame** games = getAllGames(&numGames, sqlstatementhandle);
  printf("Video Games Database\n");
  printf("==========================================\n");
  for (i = 0; i < numGames; i++) {
    printGame(games[i]);
  }

  // Cleans up connection before exiting
  finishConnection(sqlenvhandle, sqlstatementhandle, sqlconnectionhandle);
}
