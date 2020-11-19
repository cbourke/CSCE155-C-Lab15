
/**
 * Extracts an error (if any) from the given SQL handle and prints
 * it to the standard output.  Pass a message or function name to
 * distinguish where the error occurred.
 * 
 * See http://www.easysoft.com/developer/languages/c/odbc_tutorial.html#connect_full
 */
void extractError(char *functionName, SQLHANDLE handle, SQLSMALLINT type);

/**
 * Closes the SQL connection and cleans up resources.
 */
void finishConnection(SQLHANDLE envHandle, SQLHANDLE statementHandle, SQLHANDLE connectionHandle);

/**
 * Opens a database connection using the given connection
 * string.
 */
int openConnectionVerbose(SQLHANDLE handle, SQLCHAR* connectionInfo);

/**
 * Initializes an SQL handle (connection)
 */
int setupConnection(SQLHANDLE* sqlenvhandle, SQLHANDLE* sqlconnectionhandle,
                    SQLHANDLE* sqlstatementhandle, SQLCHAR* connectionInfo);
