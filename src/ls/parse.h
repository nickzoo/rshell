#ifndef PARSE_H
#define PARSE_H

#include <string>		//std::string
#include <vector>		//std::vector

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

#define BLUE "\x1b[34m"
#define GREEN "\x1b[32m"
#define BLACK "\x1b[0m"

struct File {
	std::string name;
	std::string path;
	std::string color;
};

int parse(int argc, const char *argv[],
		  std::vector<File>& regular_files,
		  std::vector<File>& directories,
		  int& flags);

#endif
