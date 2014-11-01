#include <iostream> //std::cerr
#include <vector> //std::vector
#include <stdio.h> //perror
#include <sys/stat.h> //st_mode, st_mtime, S_IFDIR, S_IRUSR, ...
#include "parse.h"

int parse(int argc, const char *argv[],
		  std::vector<const char*>& files,
		  std::vector<const char*>& directories,
		  int& flags) {
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			for (int j = 1; argv[i][j] != 0; ++j) {
				if (argv[i][j] == 'a')
					flags |= FLAG_a;
				else if (argv[i][j] == 'l')
					flags |= FLAG_l;
				else if (argv[i][j] == 'R')
					flags |= FLAG_R;
				else {
					std::cerr << "ls: illegal option: -- "
						<< argv[i][j] << std::endl;
					return 1;
				}
			}
		}
		else {
			struct stat s;
			if (stat(argv[i], &s) == 0) {
				if (s.st_mode & S_IFDIR)
					directories.push_back(argv[i]);
				else
					files.push_back(argv[i]);
			}
			else {
				std::cerr << "ls: " << argv[i] << ": ";
				perror("");
				return 1;
			}
		}
	}
	return 0;
}
