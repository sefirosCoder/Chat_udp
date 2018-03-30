CC = gcc
CFLAGS = -std=gnu99 -D_FORTIFY_SOURCE -Wall -Wconversion -Werror -Wextra -Wpedantic -pthread
LDFLAGS = -pthread

all: client serveur 

client: client.o

client.o: client.c header.h

serveur: serveur.o

serveur.o: serveur.c header.h

clean:
	$(RM) *.o
