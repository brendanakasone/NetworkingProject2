#################################################################
##
## FILE:	Makefile
## PROJECT:	CNT 4007 Project 1 - Professor Traynor
## DESCRIPTION: Compile Project 1
##
#################################################################

CC=gcc

OS := $(shell uname -s)

# Extra LDFLAGS if Solaris
ifeq ($(OS), SunOS)
	LDFLAGS=-lsocket -lnsl
    endif

all: client server 

client: client.c
	gcc -o client client.c -pthread

server: server.c
	gcc -o server server.c -pthread

clean:
	    rm -f client server *.o

