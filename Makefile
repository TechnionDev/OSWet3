#
# To compile, type "make" or make "all"
# To remove files, type "make clean"
#
OBJS = server.o request.o segel.o client.o queue.o Thread_pool.o globalConnectionsCounter.o
TARGET = server

CC = gcc
CFLAGS = -g -Wall

LIBS = -lpthread 

.SUFFIXES: .c .o 

all: server client output.cgi
	-mkdir -p public
	-cp output.cgi favicon.ico home.html public

server: server.o request.o segel.o queue.o Thread_pool.o globalConnectionsCounter.o
	$(CC) $(CFLAGS) -o server server.o request.o segel.o queue.o Thread_pool.o globalConnectionsCounter.o $(LIBS)

client: client.o segel.o
	$(CC) $(CFLAGS) -o client client.o segel.o

output.cgi: output.c
	$(CC) $(CFLAGS) -o output.cgi output.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm -f $(OBJS) server client output.cgi
	-rm -rf public


zip:
	zip submission.zip server.c request.c segel.c queue.c Thread_pool.c server.h request.h segel.h queue.h Thread_pool.h Makefile
