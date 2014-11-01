#include <iomanip> //std::setw
#include <iostream> //std::cout, std::cerr
#include <sstream> //std::stringstream
#include <string> //std::string
#include <vector> //std::vector
#include <dirent.h> //dirent, opendir, readdir
#include <errno.h> //errno
#include <grp.h> //getgrgid
#include <pwd.h> //getpwuid
#include <stdio.h> //perror, putchar
#include <string.h> //strerror
#include <sys/stat.h> //st_mode, st_mtime, S_IFDIR, S_IRUSR, ...
#include <time.h> //time_t, tm
#include "parse.h" //FLAG_a, FLAG_l, FLAG_R
#include "execute.h"

void execute(const std::vector<const char*> files,
			 const std::vector<const char*> directories,
			 int flags) {
	print_files(files);
	if (files.empty()) {
		if (directories.empty())
			print_directory(".", flags);
		else if (directories.size() == 1)
			print_directory(directories[0], flags);
		else {
			std::cout << directories[0] << ": " << std::endl;
			print_directory(directories[0], flags);
			for (size_t i = 1; i < directories.size(); ++i)
				print_directory(directories[i], flags, true);
		}
	}
	else {
		for (size_t i = 0; i < directories.size(); ++i)
			print_directory(directories[i], flags, true);
	}
}

void print_directory(const char *directory, int flags, bool extra) {
	DIR *dir_ptr = opendir(directory);
	if (dir_ptr == NULL) {
		perror("opendir");
		return;
	}
	if (extra)
		std::cout << std::endl << directory << ": " << std::endl;
	std::vector<const char*> files;
	std::vector<std::string> directory_paths;
	std::vector<std::string> paths;
	dirent *entry;
	while ((entry = readdir(dir_ptr))) {
		if (entry->d_name[0] != '.' || (flags & FLAG_a)) {
			std::string path = directory;
			path += "/" + (std::string)entry->d_name;
			paths.push_back(path);
			files.push_back(entry->d_name);
			struct stat s;
			if (stat(path.c_str(), &s) == 0) {
				if (flags & FLAG_R) { //track directories if recursing
					if (s.st_mode & S_IFDIR)
						directory_paths.push_back(path);
				}
			}
			else {
				perror("stat");
				return;
			}
		}
	}
	if (errno) {
		perror("readdir");
		return;
	}
	if (closedir(dir_ptr) == -1) {
		perror("closedir");
		return;
	}
	if (flags & FLAG_l)
		print_long(paths, files);
	else
		print_files(files);
	if (flags & FLAG_R) {
		for (size_t i = 0; i < directory_paths.size(); ++i) {
			print_directory(directory_paths[i].c_str(), flags, true);
		}
	}
}

void print_files(std::vector<const char*> files) {
	for (size_t i = 0; i < files.size(); ++i)
		std::cout << files[i] << std::endl;
}

void print_long(std::vector<std::string> paths,
				std::vector<const char*> files) {
	const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	std::vector<std::string> permissions;
	std::vector<nlink_t> links;
	std::vector<const char*> users;
	std::vector<const char*> groups;
	std::vector<off_t> sizes;
	size_t link_max = 0;
	size_t user_max = 0;
	size_t group_max = 0;
	size_t size_max = 0;
	std::vector<std::string> dates;
	for (size_t i = 0; i < paths.size(); ++i) {
		struct stat s;
		if (stat(paths[i].c_str(), &s) == 0) {
			std::stringstream ss;
			std::string permission;
			permission += (s.st_mode & S_IFDIR ? 'd' : '-');
			permission += (s.st_mode & S_IRUSR ? 'r' : '-');
			permission += (s.st_mode & S_IWUSR ? 'w' : '-');
			permission += (s.st_mode & S_IXUSR ? 'x' : '-');
			permission += (s.st_mode & S_IRGRP ? 'r' : '-');
			permission += (s.st_mode & S_IWGRP ? 'w' : '-');
			permission += (s.st_mode & S_IXGRP ? 'x' : '-');
			permission += (s.st_mode & S_IROTH ? 'r' : '-');
			permission += (s.st_mode & S_IWOTH ? 'w' : '-');
			permission += (s.st_mode & S_IXOTH ? 'x' : '-');
			permissions.push_back(permission);
			ss << s.st_nlink; std::string slink = ss.str(); ss.str("");
			if (link_max < slink.size()) link_max = slink.size();
			links.push_back(s.st_nlink);
			const char* user = getpwuid(s.st_uid)->pw_name;
			if (user_max < strlen(user)) user_max = strlen(user);
			users.push_back(user);
			const char* group = getgrgid(s.st_gid)->gr_name;
			if (group_max < strlen(group)) group_max = strlen(group);
			groups.push_back(group);
			ss << s.st_size; std::string ssize = ss.str(); ss.str("");
			if (size_max < ssize.size()) size_max = ssize.size();
			std::cout << "size max: " << size_max << std::endl;
			sizes.push_back(s.st_size);
			time_t mtime = s.st_mtime;
			tm *ltm = localtime(&mtime);
			std::string date;
			date += months[ltm->tm_mon]; date += ' ';
			if (ltm->tm_mday < 10) date += ' ';
			ss << ltm->tm_mday << ' '; date += ss.str(); ss.str("");
			if (ltm->tm_hour < 10) date += '0';
			ss << ltm->tm_hour << ':'; date += ss.str(); ss.str("");
			if (ltm->tm_min < 10) date += '0';
			ss << ltm->tm_min; date += ss.str(); ss.str("");
			dates.push_back(date);
		}
		else {
			perror("stat");
			return;
		}
	}
	for (size_t i = 0; i < permissions.size(); ++i) {
		std::cout << permissions[i] << ' '
				  << std::setw(link_max) << links[i] << ' '
				  << std::setw(user_max) << users[i] << ' '
				  << std::setw(group_max) << groups[i] << ' '
				  << std::setw(size_max) << sizes[i] << ' '
				  << dates[i] << ' ';
		struct stat s;
		if (stat(paths[i].c_str(), &s) == 0) {
			if (s.st_mode & S_IFDIR) std::cout << "\x1b[34m";
			else if (s.st_mode & S_IXUSR) std::cout << "\x1b[32m";
			std::cout << files[i] << "\x1b[0m" << std::endl;
		}
		else {
			perror("stat");
			return;
		}
	}
}
