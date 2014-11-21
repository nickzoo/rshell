#ifndef RSHELL_H
#define RSHELL_H

#include <string>	//string, getline
#include <vector>	//vector
using namespace std;

int parse(string& line);
void parse_escape(string& line, size_t& i, string& token);
void parse_quotes(string& line, size_t& i, string& token);
int parse_ioredirect(string& line, size_t& i, size_t& j, string& token);
int parse_end(size_t& i, size_t& j, string& token);
int parse_conditional(string& line, size_t& i, size_t& j, string& token);
int parse_pipe(string& line, size_t& i, size_t& j, string& token);
void more_input(string& line);
int execute();
std::vector<char**> c_compatible();
void c_delete(vector<char**>& c_commands);

#endif
