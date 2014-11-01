CC = g++
CFLAGS = -Wall -Werror -ansi -pedantic
RSHELL_SOURCES = src/rshell/rshell.cpp src/rshell/parse.cpp src/rshell/execute.cpp
RSHELL_EXECUTABLE = bin/rshell
LS_SOURCES = src/ls/ls.cpp src/ls/parse.cpp src/ls/execute.cpp
LS_EXECUTABLE = bin/ls

all: rshell ls

rshell:
	test -d bin || mkdir bin
	$(CC) $(CFLAGS) $(RSHELL_SOURCES) -o $(RSHELL_EXECUTABLE)

ls:
	test -d bin || mkdir bin
	$(CC) $(CFLAGS) $(LS_SOURCES) -o $(LS_EXECUTABLE)

clean:
	rm -rf bin
