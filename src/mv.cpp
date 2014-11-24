#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include "rm.h"
using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("usage: mv [file1] [file2]\n");
		return 1;
	}
	struct stat s_source;
	struct stat s_dest;
	if (stat(argv[2], &s_dest) == -1) {
		if (link(argv[1], argv[2]) == -1) {
			perror("link");
			return 1;
		}
		if (stat(argv[1], &s_source) == -1) {
			perror("stat");
			return 1;
		}
		if (s_source.st_mode & S_IFDIR) {
			rmdir_recursive(argv[1]);
		}
		else {
			if (unlink(argv[1]) == -1) {
				perror("unlink");
				return 1;
			}
		}
	}
	else {
		if (s_dest.st_mode & S_IFDIR) {
			string path = argv[2];
			if (path[path.length()-1] != '/') path += '/';
			path += argv[1];
			if (link(argv[1], path.c_str()) == -1) {
				perror("link");
				return 1;
			}
			if (unlink(argv[1]) == -1) {
				perror("unlink");
				return 1;
			}
		}
		else printf("file already exists.\n");
	}
	return 0;
}
