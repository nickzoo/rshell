#include <vector>
#include "parse.h"
#include "execute.h"

int main(int argc, const char *argv[]) {
	std::vector<File> regular_files;
	std::vector<File> directories;
	int flags = 0;
	if (parse(argc, argv, regular_files, directories, flags))
		return 1;
	execute(regular_files, directories, flags);
	return 0;
}
