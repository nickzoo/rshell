#ifndef RM_H
#define RM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <vector>

void rmdir_recursive(const std::string& dirpath) {
	std::vector<std::string> directories;
	DIR *dirptr = opendir(dirpath.c_str());
	if (dirptr == NULL) {
		perror("opendir");
		exit(1);
	}
	dirent *entry;
	while (entry = readdir(dirptr)) {
		std::string entryname = entry->d_name;
		if (entryname == "." || entryname == "..") continue;
		std::string filepath = dirpath + entryname;
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

#endif
