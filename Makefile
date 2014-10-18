CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic
SOURCES = src/rshell.cpp
OBJECTS = bin/rshell.o
EXECUTABLE = bin/rshell

all: rshell

rshell:
	test -d bin || mkdir bin
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

clean:
	rm -rf bin
