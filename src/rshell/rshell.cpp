#include <iostream> //std::cout, std::cin
#include <string> //std::string, std::getline
#include <vector> //std::vector
#include <stdio.h> //perror
#include <unistd.h> //getlogin_r, gethostname
#include "parse.h" //parse
#include "execute.h" //execute

#define MAX_LENGTH 256

int main() {
	char login[MAX_LENGTH];
	char hostname[MAX_LENGTH];
	while (true) {
		if (getlogin_r(login, MAX_LENGTH) != 0) {
			perror("getlogin_r");
			return 1;
		}
		if (gethostname(hostname, MAX_LENGTH) != 0) {
			perror("gethostname");
			return 1;
		}
		std::cout << login << "@" << hostname << "$ ";
		std::string line;
		std::getline(std::cin, line);
		std::vector< std::vector<std::string> > parsed;
		if (parse(line, parsed)) { //parse returns 1 on error
			continue;
		}
		if (execute(parsed)) { //execute returns 1 on exit
			break;
		}
	}
	return 0;
}
