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

  // at this point, we have our sql connection handle; we can use it in the
  // games API...

  // TODO: 1. Define string variables to hold your game's name, publisher,
  // platform

  // TODO: 2. Check that your game does not already exist, quit if it does

  // TODO: 3. Check that a record for your publisher doesn't already exist,
  //         if it does use it, otherwise add it

  // TODO: 4. Check that a record for your platform doesn't already exist,
  //         if it does use it, otherwise add it

  // TODO: 5. Add the video game to the database

  // TODO: 6. Load the game from the database

  // TODO: 7. Associate your game with the platform with a particular release
  // year

  // Cleans up connection before exiting
  finishConnection(sqlenvhandle, sqlstatementhandle, sqlconnectionhandle);
}
