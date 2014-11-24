#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

void rmdir_recursive(const string& dirpath) {
	vector<string> directories;
	DIR *dirptr = opendir(dirpath.c_str());
	if (dirptr == NULL) {
		perror("opendir");
		exit(1);
	}
	dirent *entry;
	while (entry = readdir(dirptr)) {
		string entryname = entry->d_name;
		if (entryname == "." || entryname == "..") continue;
		string filepath = dirpath + entryname;
		struct stat s;
		if (stat(filepath.c_str(), &s) == -1) {
			perror("stat");
			exit(1);
		}
		if (s.st_mode & S_IFDIR) {
			filepath += '/';
			directories.push_back(filepath);
		}
		else {
			if (unlink(filepath.c_str()) == -1) {
				perror("unlink");
				exit(1);
			}
		}
	}
	for (int i = 0; i < directories.size(); ++i)
		rmdir_recursive(directories[i]);
	if (rmdir(dirpath.c_str()) == -1) {
		perror("rmdir");
		exit(1);
	}
}

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
