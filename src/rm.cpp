#include <string.h>
#include "rm.h"
using namespace std;

int main(int argc, char *argv[]) {
	bool FLAGr = false;
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-r") == 0) FLAGr = true;
	}
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') continue;
		struct stat s;
		if (stat(argv[i], &s) == -1) {
			perror("stat");
			return 1;
		}
		if (s.st_mode & S_IFDIR) {
			if (!FLAGr) {
				printf("set -r flag to delete directories.\n");
				continue;
			}
			string dirpath = argv[i];
			if (dirpath[dirpath.length()-1] != '/') dirpath += '/';
			rmdir_recursive(dirpath);
		}
		else {
			if (unlink(argv[i]) == -1) {
				perror("unlink");
				return 1;
			}
		}
	}
	return 0;
}
