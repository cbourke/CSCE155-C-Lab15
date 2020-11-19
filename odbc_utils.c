#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "games.h"
#include "odbc_utils.h"

void printGame(VideoGame* game) {
  printf("Name: %s\n\tID: %d\n\tPublisher: ", game->name, game->id);

  // Publisher may be unknown
  if (game->publisher != NULL)
    printf("%s\n", game->publisher->name);
  else
    printf("Unknown\n");

  // Iterates over this games platforms
  for (int i = 0; i < game->numPlatforms; i++) {
    printf("\tReleased on platform %s in %d\n", game->platforms[i]->name,
           game->years[i]);
  }
}

int setupConnection(SQLHANDLE* sqlenvhandle, SQLHANDLE* sqlconnectionhandle,
                    SQLHANDLE* sqlstatementhandle, SQLCHAR* connectionInfo) {
  int errorFlag = 1;

  // Allocates handle for the database
  if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, sqlenvhandle) !=
      SQL_SUCCESS) {
    fprintf(stderr, "Could not allocate handle for the database\n");
    errorFlag = 0;
  }

  // Sets up environment flags
  else if (SQLSetEnvAttr(*sqlenvhandle, SQL_ATTR_ODBC_VERSION,
                         (SQLPOINTER)SQL_OV_ODBC3, 0) != SQL_SUCCESS) {
    fprintf(stderr, "Error setting up environment flags\n");
    errorFlag = 0;
  }

  // Creates handle to the database
  else if (SQLAllocHandle(SQL_HANDLE_DBC, *sqlenvhandle, sqlconnectionhandle) !=
           SQL_SUCCESS) {
    fprintf(stderr, "Could not create a handle to the database\n");
    errorFlag = 0;
  }

  // Open database connection
  else if (!openConnectionVerbose(*sqlconnectionhandle,
                                  (SQLCHAR*)connectionInfo)) {
    fprintf(stderr, "Could not establish a database connection\n");
    errorFlag = 0;
  }

  // Creates a statement variable
  else if (SQLAllocHandle(SQL_HANDLE_STMT, *sqlconnectionhandle,
                          sqlstatementhandle) != SQL_SUCCESS) {
    fprintf(stderr, "Could not create an SQL statement variable\n");
    errorFlag = 0;
  }

  return errorFlag;
}

void finishConnection(SQLHANDLE envHandle, SQLHANDLE statementHandle,
                      SQLHANDLE connectionHandle) {
  SQLFreeHandle(SQL_HANDLE_STMT, statementHandle);
  SQLDisconnect(connectionHandle);
  SQLFreeHandle(SQL_HANDLE_DBC, connectionHandle);
  SQLFreeHandle(SQL_HANDLE_ENV, envHandle);
}

int openConnectionVerbose(SQLHANDLE sqlconnectionhandle,
                          SQLCHAR* connectionInfo) {
  SQLCHAR retconstring[1024];

  // Connects to the databse with the given info
  RETCODE returnCode =
      SQLDriverConnect(sqlconnectionhandle, NULL, connectionInfo, SQL_NTS,
                       retconstring, 1024, NULL, SQL_DRIVER_NOPROMPT);
  // Did the connection succeed?  Print message if not.
  switch (returnCode) {
    case SQL_SUCCESS_WITH_INFO:
      extractError("openConnectionVerbose", sqlconnectionhandle,
                   SQL_HANDLE_DBC);
      break;

    case SQL_INVALID_HANDLE:
    case SQL_ERROR:
      extractError("openConnectionVerbose", sqlconnectionhandle,
                   SQL_HANDLE_DBC);
      return -1;

    default:
      break;
  }

  return 1;
}

// From
// http://www.easysoft.com/developer/languages/c/odbc_tutorial.html#connect_full
void extractError(char* fn, SQLHANDLE handle, SQLSMALLINT type) {
  SQLINTEGER i = 0;
  SQLINTEGER native;
  SQLCHAR state[7];
  SQLCHAR text[256];
  SQLSMALLINT len;
  SQLRETURN ret;

  // Gets last message in the diagnostic record from SQL
  fprintf(stderr,
          "The driver reported the following diagnostics whilst running %s",
          fn);
  do {
    ret = SQLGetDiagRec(type, handle, ++i, state, &native, text, sizeof(text),
                        &len);
    if (SQL_SUCCEEDED(ret)) printf("%s:%d:%d:%s\n", state, i, native, text);
  } while (ret == SQL_SUCCESS);
}
