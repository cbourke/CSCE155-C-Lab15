
/**
 * A structure to model a Publisher
 */
typedef struct {
  int id;
  char name[1024];
} Publisher;

/**
 * A structure to model a Platform
 */
typedef struct {
  int id;
  char name[1024];
} Platform;

/**
 * A structure to model a VideoGame
 */
typedef struct {
  int id;
  int numPlatforms;
  char name[1024];
  int* years;
  Platform** platforms;
  Publisher* publisher;
} VideoGame;

/**
 * Adds a video game record to the database with the
 * given name and associating it with the publisher
 * identified by the given publisherID.
 */
SQLRETURN addVideoGame(char* name, int publisherID, SQLHANDLE handle);

/**
 * Adds a plaform record to the database with the
 * given name.
 */
SQLRETURN addPlatform(char* name, SQLHANDLE handle);

/**
 * Adds a publisher record to the database with the
 * given name.
 */
SQLRETURN addPublisher(char* name, SQLHANDLE handle);

/**
 * Adds an availability record to the database, associating
 * a game with a particular platform (both identified by the
 * given IDs).  Models the fact that the game was released
 * on a particular platform in the provided publishYear.
 */
SQLRETURN addAvailability(int gameID, int platformID, int publishYear,
                          SQLHANDLE handle);

/**
 * Returns an instance of a VideoGame structure with 
 * field values retrieved from the database based on
 * the given name.  If no such record exists, returns 
 * NULL.
 */
VideoGame* getGame(char* name, SQLHANDLE handle);

/**
 * Returns an instance of a Publisher structure with 
 * field values retrieved from the database based on
 * the given name.  If no such record exists, returns 
 * NULL.
 */
Publisher* getPublisher(char* name, SQLHANDLE handle);

/**
 * Returns an instance of a Platform structure with 
 * field values retrieved from the database based on
 * the given name.  If no such record exists, returns 
 * NULL.
 */
Platform* getPlatform(char* name, SQLHANDLE handle);

/**
 * Fills out additional fields in the given VideoGame 
 * structure based on the availability and platform 
 * tables.
 */
void getAvailability(VideoGame* game, SQLHANDLE handle);

/**
 * Returns an array of VideoGame structures of all
 * game records in the database.  The size of the
 * resulting array is put in the pass-by-reference
 * numGames variable.
 */
VideoGame** getAllGames(int* numGames, SQLHANDLE handle);
