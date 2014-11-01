#ifndef PARSE_H
#define PARSE_H

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

/*parses input into non-directory files and directories and records flags,
ignoring hidden files unless -a flag raised*/
int parse(int argc, const char *argv[],
		  std::vector<const char*>& files,
		  std::vector<const char*>& directories,
		  int& flags);

#endif
