
# CSCE 155E 
## Lab 15.0 Worksheet

Name(s) and Login(s):



1. Look over the code provided in the `games.h` and `games.c` 
   header and source file and answer the following.

a) Which function should be used to add a platform to the database 
  and what are its parameters?


b) What type of SQL query is used to add records to the database 
  (hint: look at the `addPlatform`, `addVideoGame`, `addPublisher` 
  functions)?


c) What type of SQL query is used to retrieve records from the 
  database (hint: look at the `getGame`, `getPlatform`, etc. functions).


2. Data Integrity

a) What error messages did you get and why?


b) A bad database design may not have properly defined the foreign 
   keys between the game and publisher tables, instead allowing the bad 
   game data that we attempted to insert into the database.  What 
   consequences could this have for a larger application?


c) Consider the following: two programs or users use our API to add 
  two different games to the database, both PlayStation 3 games.  
  However, the first user inserts the platform as "PS3" and the second 
  inserts it as "PlayStation III".  Would this result in an error?  
  What would the consequences of this be for a larger application?


d) Say that we wanted to remove a publisher from the database.  
   What would we need to do before we could remove the record?







