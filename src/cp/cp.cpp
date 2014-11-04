#include <fstream>		//std::ifstream, std::ofstream
#include <iostream>		//std::cout, std::cerr
#include <errno.h>		//errno
#include <fcntl.h>		//O_CREAT, O_RDONLY, O_WRONLY
#include <stdio.h>		//perror
#include <string.h>		//strcmp
#include <sys/stat.h>	//stat
#include <time.h>
#include <unistd.h>		//read, write, close
#include "Timer.h"

void copy1(char *i_name, char *o_name) {
	std::ifstream in(i_name);
	std::ofstream out(o_name);
	char c;
	while ((c = in.get()) != EOF)
		out.put(c);
	in.close();
	out.close();
}

void copy2(char *i_name, char *o_name) {
	int i_file = open(i_name, O_RDONLY);
	if (i_file == -1) {
		perror("open");
		return;
	}
	int o_file = open(o_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (o_file == -1) {
		perror("open");
		return;
	}
	char c;
	while (read(i_file, &c, 1)) {
		if (errno) {
			perror("read");
			break;
		}
		if (write(o_file, &c, 1) == -1) {
			perror("write");
			break;
		}
	}
	i_file = close(i_file);
	o_file = close(o_file);
	if (i_file == -1 || o_file == -1) {
		perror("close");
		return;
	}
}

void copy3(char *i_name, char *o_name) {
	int i_file = open(i_name, O_RDONLY);
	if (i_file == -1) {
		perror("open");
		return;
	}
	int o_file = open(o_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
	if (o_file == -1) {
		perror("open");
		return;
	}
	char buf[BUFSIZ];
	size_t bytes_read;
	while ((bytes_read = read(i_file, buf, BUFSIZ))) {
		if (errno) {
			perror("read");
			break;
		}
		if (write(o_file, buf, bytes_read) == -1) {
			perror("write");
			break;
		}
	}
	i_file = close(i_file);
	o_file = close(o_file);
	if (i_file == -1 || o_file == -1) {
		perror("close");
		return;
	}
}

void time_functions(char *i_name, char *o_name) {
	std::cout << std::endl;
	Timer t;

	t.start();
	copy1(i_name, o_name);
	t.stop();
	std::cout << "ifstream::get and ostream::put one character at a time\n";
	t.print();
	std::cout << std::endl;
	remove(o_name);

	t.start();
	copy2(i_name, o_name);
	t.stop();
	std::cout << "read and write one character at a time\n";
	t.print();
	std::cout << std::endl;
	remove(o_name);

	t.start();
	copy3(i_name, o_name);
	t.stop();
	std::cout << "read and write one buffer at a time\n";
	t.print();
	std::cout << std::endl;
}

inline bool file_exists(const char *name) {
	struct stat s;
	bool ret = (stat(name, &s) == 0);
	errno = 0;
	return ret;
}

int main(int argc, char *argv[]) {
	if (argc != 3 && argc != 4) {
		std::cerr << "illegal number of arguments\n";
		return 1;
	}
	if ((argc == 3 && file_exists(argv[2])) ||
		(argc == 4 && file_exists(argv[3]))) {
		std::cerr << "destination file already exists\n";
		return 1;
	}
	if (argc == 4) {
		if (strcmp(argv[1], "-t") == 0) {
			time_functions(argv[2], argv[3]);
		}
		else {
			std::cerr << "illegal usage\n";
			return 1;
		}
	}
	else copy3(argv[1], argv[2]);
	return 0;
}
