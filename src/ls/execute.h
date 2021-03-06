#ifndef EXECUTE_H
#define EXECUTE_H

void execute(std::vector<File>& regular_files,
			 std::vector<File>& directories,
			 int flags);

void print_directory(const File& directory, int flags, bool extra=false);

void print_files(const std::vector<File>& files, int flags);

void print_files_long(const std::vector<File>& files);

bool by_name(const File& left, const File& right);

#endif
