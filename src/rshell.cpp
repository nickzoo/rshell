#include <iostream>
#include <string>
#include <vector>
#include <stdio.h> //perror
#include <unistd.h> //getlogin_r, gethostname
#include "rparser.h" //parse
#include "rexecute.h" //execute

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
		if (std::cin.eof())
			break;
		getline(std::cin, line);
		std::vector< std::vector<std::string> > parsed;
		int error = parse(line, parsed);
		if (error) {
			continue;
		}
		int exit_now = execute(parsed);
		if (exit_now) {
			break;
		}
	}
	return 0;
}
