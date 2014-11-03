#include <fstream>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
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
	double wallclock_time, user_time, system_time;

	t.start();
	copy1(i_name, o_name);
	t.elapsedWallclockTime(wallclock_time);
	t.elapsedUserTime(user_time);
	t.elapsedSystemTime(system_time);
	std::cout << "wallclock time: " << wallclock_time << std::endl;
	std::cout << "user time: " << user_time << std::endl;
	std::cout << "system time: " << system_time << std::endl;
	std::cout << std::endl;
	remove(o_name);

	t.start();
	copy2(i_name, o_name);
	t.elapsedWallclockTime(wallclock_time);
	t.elapsedUserTime(user_time);
	t.elapsedSystemTime(system_time);
	std::cout << "wallclock time: " << wallclock_time << std::endl;
	std::cout << "user time: " << user_time << std::endl;
	std::cout << "system time: " << system_time << std::endl;
	std::cout << std::endl;
	remove(o_name);

	t.start();
	copy3(i_name, o_name);
	t.elapsedWallclockTime(wallclock_time);
	t.elapsedUserTime(user_time);
	t.elapsedSystemTime(system_time);
	std::cout << "wallclock time: " << wallclock_time << std::endl;
	std::cout << "user time: " << user_time << std::endl;
	std::cout << "system time: " << system_time << std::endl;
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
		std::cerr << "incorrect number of arguments\n";
		return 1;
	}
	if (file_exists(argv[2])) {
		std::cerr << "destination file already exists\n";
		return 1;
	}
	if (argc == 4) time_functions(argv[1], argv[2]);
	else copy3(argv[1], argv[2]);
	return 0;
}
