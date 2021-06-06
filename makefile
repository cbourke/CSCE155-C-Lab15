#
# makefile for Lab 15 - ODBC
#
# ignore the SQLCHAR* implicit casts
CC = gcc -std=gnu99 -Wall -Wextra -Wno-pointer-sign -Wno-unused-parameter
ODBC_LIB = -lodbc

.PHONY: clean
.PHONY: all

all: listGames insertGame

listGames: databaseInfo.h games.o odbc_utils.o listGames.c
	$(CC) -o listGames games.o odbc_utils.o listGames.c $(ODBC_LIB)

insertGame: databaseInfo.h games.o odbc_utils.o insertGame.c
	$(CC) -o insertGame games.o odbc_utils.o insertGame.c $(ODBC_LIB)

odbc_utils.o: odbc_utils.c odbc_utils.h
	$(CC) -c -o odbc_utils.o odbc_utils.c

games.o: games.h games.c
	$(CC) -c -o games.o games.c

clean:
	rm *.o *~
