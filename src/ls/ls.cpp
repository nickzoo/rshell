#include <iostream>
#include <string>
#include <vector>
#include "parse.h"
#include "execute.h"

int main(int argc, const char *argv[]) {
	std::vector<const char*> files;
	std::vector<const char*> directories;
	int flags = 0;
	//parse returns 1 on error, i.e. file not found or illegal option
	if (parse(argc, argv, files, directories, flags))
		return 1;
	execute(files, directories, flags);
	return 0;
}
