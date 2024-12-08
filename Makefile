CC = gcc
CFLAGS = -std=gnu99 -Wall -lm -DDEBUG

all: client server

client: kermit
	$(CC) $(CFLAGS) client.c kermit.o -o client

server: kermit
	$(CC) $(CFLAGS) server.c kermit.o -o server 

kermit:
	$(CC) $(CFLAGS) -c kermit.c

clean:
	rm -f kermit.o

purge: clean
	rm -f client server
