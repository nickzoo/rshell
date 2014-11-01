#ifndef EXECUTE_H
#define EXECUTE_H

//uses fork, execvp, and waitpid to execute commands
int execute(const std::vector< std::vector<std::string> >& parsed);

/*parsed vector of strings must be converted to array of cstrings for use
with execvp*/
std::vector<char**> c_compatible(const std::vector< std::vector<std::string> >& parsed);

//frees dynamically allocated array of cstrings created by c_compatible
void c_delete(const std::vector< std::vector<std::string> >& parsed,
		std::vector<char**>& c_parsed);

#endif
