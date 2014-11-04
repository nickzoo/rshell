#include <algorithm>	//std::sort
#include <iomanip>		//std::setw
#include <iostream>		//std::cout, std::cerr
#include <sstream>		//std::stringstream
#include <string>		//std::string
#include <vector>		//std::vector
#include <dirent.h>		//dirent, opendir, readdir
#include <errno.h>		//errno
#include <grp.h>		//getgrgid
#include <pwd.h>		//getpwuid
#include <stdio.h>		//perror
#include <string.h>		//strlen
#include <sys/ioctl.h>	//winsize, ioctl
#include <sys/stat.h>	//st_mode, st_mtime, S_IFDIR, S_IRUSR, ...
#include <time.h>		//time_t, tm
#include "parse.h"		//File, FLAG_a, FLAG_l, FLAG_r
#include "execute.h"

void execute(std::vector<File>& regular_files,
			 std::vector<File>& directories,
			 int flags) {
	std::sort(regular_files.begin(), regular_files.end(), by_name);
	std::sort(directories.begin(), directories.end(), by_name);
	if (!regular_files.empty()) {
		print_files(regular_files, flags);
		for (size_t i = 0; i < directories.size(); ++i)
			print_directory(directories[i], flags, true);
	}
	else {
		if (directories.empty()) {
			File current; current.name = "."; current.path = ".";
			print_directory(current, flags);
		}
		else if (directories.size() == 1)
			print_directory(directories[0], flags);
		else {
			std::cout << directories[0].path << ": " << std::endl;
			print_directory(directories[0], flags);
			for (size_t i = 1; i < directories.size(); ++i)
				print_directory(directories[i], flags, true);
		}
	}
}

void print_directory(const File& directory, int flags, bool extra) {
	DIR *dir_ptr = opendir(directory.path.c_str());
	if (dir_ptr == NULL) {
		perror("opendir");
		return;
	}
	if (extra)
		std::cout << std::endl << directory.path << ": " << std::endl;
	std::vector<File> files;
	std::vector<File> directories;
	dirent *entry;
	while ((entry = readdir(dir_ptr))) {
		if (entry->d_name[0] != '.' || (flags & FLAG_a)) {
			File file;
			std::string path = directory.path;
			path += "/" + (std::string)entry->d_name;
			file.name = entry->d_name;
			file.path = path;
			struct stat s;
			if (stat(file.path.c_str(), &s) == 0) {
				if (s.st_mode & S_IFDIR) {
					file.name += '/';
					file.color += BLUE;
					if (flags & FLAG_R &&
							file.name != "./" && file.name != "../") {
						directories.push_back(file);
					}
				}
				else if (s.st_mode & S_IXUSR) {
					file.name += '*';
					file.color += GREEN;
				}
				else file.color += BLACK;
				files.push_back(file);
			}
			else
				perror("stat");
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
	std::sort(files.begin(), files.end(), by_name);
	std::sort(directories.begin(), directories.end(), by_name);
	print_files(files, flags);
	for (size_t i = 0; i < directories.size(); ++i)
		print_directory(directories[i], flags, true);
}

void print_files(const std::vector<File>& files, int flags) {
	if (flags & FLAG_l) {
		print_files_long(files); return;
	}
	struct winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	int max_width = w.ws_col;
	unsigned n_rows = 1;
	std::vector<unsigned> column_width(files.size()+1);
	while (n_rows < files.size()) {
		int column = -1;
		for (size_t i = 0; i < files.size(); ++i) {
			if (i % n_rows == 0) ++column;
			if (column_width[column] < files[i].name.size()+2)
				column_width[column] = files[i].name.size()+2;
		}
		int total_width = 0;
		for (size_t i = 0; column_width[i] != 0; ++i)
			total_width += column_width[i];
		if (total_width <= max_width) break;
		for (size_t i = 0; column_width[i] != 0; ++i)
			column_width[i] = 0;
		++n_rows;
	}
	for (size_t i = 0; i < n_rows; ++i) {
		for (size_t j = i; j < files.size(); j += n_rows) {
			size_t column = j / n_rows;
			std::cout << files[j].color;
			struct stat s;
			if (stat(files[i].path.c_str(), &s) == 0) {
				if (s.st_mode & (S_IFDIR | S_IXUSR)) {
					std::cout << files[j].name.substr(0,
						files[j].name.size()-1) << BLACK
						<< files[j].name[files[j].name.size()-1];
				}
				else {
					std::cout << files[j].name << BLACK;
				}
				std::cout << std::setw(column_width[column] -
						files[j].name.size()) << " ";
			}
		}
		std::cout << std::endl;
	}
}

void print_files_long(const std::vector<File>& files) {
	blkcnt_t total_blocks = 0;
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
    for (size_t i = 0; i < files.size(); ++i) {
        struct stat s;
        if (stat(files[i].path.c_str(), &s) == 0) {
			total_blocks += s.st_blocks;
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
	std::cout << "total " << total_blocks << std::endl;
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << permissions[i] << ' '
                  << std::setw(link_max) << links[i] << ' '
                  << std::setw(user_max) << users[i] << ' '
                  << std::setw(group_max) << groups[i] << ' '
                  << std::setw(size_max) << sizes[i] << ' '
                  << dates[i] << ' ';
		std::cout << files[i].color << files[i].name << BLACK << std::endl;
    }
}

bool by_name(const File& left, const File& right) {
	std::string upper_left, upper_right;
	for (size_t i = 0; i < left.name.size(); ++i) {
		if (left.name[i] == '.') continue;
		upper_left += toupper(left.name[i]);
	}
	for (size_t i = 0; i < right.name.size(); ++i) {
		if (right.name[i] == '.') continue;
		upper_right += toupper(right.name[i]);
	}
	if (upper_left == upper_right) return left.name > right.name;
	else return upper_left < upper_right;
}
