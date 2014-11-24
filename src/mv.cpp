#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("usage: mv [file1] [file2]\n");
		return 1;
	}
	struct stat s;
	if (stat(argv[2], &s) == -1) {
		if (link(argv[1], argv[2]) == -1) {
			perror("link");
			return 1;
		}
		if (unlink(argv[1]) == -1) {
			perror("unlink");
			return 1;
		}
	}
	else {
		if (s.st_mode & S_IFDIR) {
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
}
