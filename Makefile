CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic
SOURCES = src/rshell.cpp src/parse.cpp src/execute.cpp
EXECUTABLE = bin/rshell

all: rshell

rshell:
	test -d bin || mkdir bin
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXECUTABLE)

clean:
	rm -rf bin
