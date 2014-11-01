#ifndef EXECUTE_H
#define EXECUTE_H

#include "parse.h"

//executes ls call given input files, directories, and flags
void execute(const std::vector<const char*> files,
			 const std::vector<const char*> directories,
			 int flags);

//prints the contents of a directory, recursively if -R flag raised
void print_directory(const char *directory, int flags, bool extra=false);

//prints files, calls print_long if "l" flag raised
void print_files(std::vector<const char*> files);

//prints files format long
void print_long(std::vector<std::string> paths,
				std::vector<const char*> files);
#endif
