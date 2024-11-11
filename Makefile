CC = gcc
CFLAGS = -std=c99 -Wall -Wno-parentheses

all: client.o server.o

client.o: client.c
	$(CC) $(CFLAGS) client.c -o client

server.o: server.c
	$(CC) $(CFLAGS) server.c -o server

clean:
	rm -f client server
