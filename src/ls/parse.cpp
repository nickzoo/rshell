#include <iostream>		//std::cerr, std::endl
#include <errno.h>		//errno
#include <stdio.h>		//perror
#include <sys/stat.h>	//stat, st_mode, st_SIFDIR, S_IXUSR, ...
#include "parse.h"		//File, FLAG_a, FLAG_l, FLAG_r

int parse(int argc, const char *argv[],
		  std::vector<File>& regular_files,
		  std::vector<File>& directories,
		  int& flags) {
	bool options = true;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && options) {
			for (int j = 1; argv[i][j] != 0; ++j) {
				if (argv[i][j] == 'a') flags |= FLAG_a;
				else if (argv[i][j] == 'l') flags |= FLAG_l;
				else if (argv[i][j] == 'R') flags |= FLAG_R;
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
				File file;
				file.name = argv[i];
				file.path = argv[i];
				if (file.name[0] == '.')
					file.color += GRAY_BACKGROUND;
				if (s.st_mode & S_IFDIR) {
					file.name += '/';
					file.color += BLUE;
					directories.push_back(file);
				}
				else {
					if (s.st_mode & S_IXUSR) {
						file.name += '*';
						file.color += GREEN;
					}
					else file.color += BLACK;
					regular_files.push_back(file);
				}
			}
			else {
				std::cerr << "ls: " << argv[i] << ": ";
				perror("");
			}
			options = false;
		}
	}
	if (errno) {
		if (regular_files.size() == 0 && directories.size() == 0)
			return 1;
		errno = 0;
	}
	return 0;
}
