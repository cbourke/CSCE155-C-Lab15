#
# makefile for Lab 15 - ODBC
#

CC = gcc 
ODBC_LIB = -lodbc

listGames: databaseInfo.h games.o odbc_utils.o listGames.c
	$(CC) $(ODBC_LIB) -o listGames games.o odbc_utils.o listGames.c

insertGame: databaseInfo.h games.o odbc_utils.o insertGame.c
	$(CC) $(ODBC_LIB) -o insertGame games.o odbc_utils.o insertGame.c

odbc_utils.o: odbc_utils.c odbc_utils.h
	$(CC) -c -o odbc_utils.o odbc_utils.c

games.o: games.h games.c
	$(CC) -c -o games.o games.c

clean:
	rm *.o *~
