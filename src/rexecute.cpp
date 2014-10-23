#include <string>
#include <vector>
#include <stdio.h> //perror
#include <stdlib.h> //exit, NULL
#include <string.h> //strcmp, memcpy
#include <sys/wait.h> //waitpid
#include <unistd.h> //fork, execvp
#include "rexecute.h"

int execute(const std::vector< std::vector<std::string> >& parsed) {
	std::vector<char**> c_parsed = c_compatible(parsed);
	int status;
	for (size_t i = 0; i < c_parsed.size(); ++i) {
		if (!c_parsed[i][0]) return 0;
		if (strcmp(c_parsed[i][0], "exit") == 0 ||
				(i > 0 && c_parsed[i][1] &&
				 strcmp(c_parsed[i][1], "exit") == 0)) {
			c_delete(parsed, c_parsed);
			return 1;
		}
		int pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			if (i == 0) {
				execvp(c_parsed[i][0], c_parsed[i]);
				perror("execvp");
				exit(1);
			}
			else {
				if (c_parsed[i][1] == NULL)
					exit(0);
				if (strcmp(c_parsed[i][0], "||") == 0 && status == 0)
					exit(0);
				if (strcmp(c_parsed[i][0], "&&") == 0 && status != 0)
					exit(0);
				execvp(c_parsed[i][1], c_parsed[i]+1);
				perror("execvp");
				exit(1);
			}
		}
		else {
			if (waitpid(pid, &status, 0) < 0) {
				perror("waitpid");
				exit(1);
			}
		}
	}
	c_delete(parsed, c_parsed);
	return 0;
}

std::vector<char**> c_compatible(const std::vector< std::vector<std::string> >& parsed) {
	std::vector<char**> c_parsed;
	for (size_t i = 0; i < parsed.size(); ++i) {
		char **argv = new char*[parsed[i].size()+1];
		size_t j;
		for (j = 0; j < parsed[i].size(); ++j) {
			argv[j] = new char[parsed[i][j].size()+1];
			strcpy(argv[j], parsed[i][j].c_str());
		}
		argv[j] = NULL;
		c_parsed.push_back(argv);
	}
	return c_parsed;
}
 
//an array of cstd::strings is necessary for execvp
void c_delete(const std::vector< std::vector<std::string> >& parsed,
		std::vector<char**>& c_parsed) {
	for (size_t i = 0; i < parsed.size(); ++i) {
		for (size_t j = 0; j < parsed[i].size(); ++j) {
			delete[] c_parsed[i][j];
		}
		delete[] c_parsed[i];
	}
}
