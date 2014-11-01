#ifndef PARSE_H
#define PARSE_H

/*parses line into a vector of commands separated by connectors;
each command is a vector of arguments*/
int parse(std::string& line,
		std::vector< std::vector<std::string> >& parsed);

/*handles \ character escape, including newline which prompts for more
input on the next line */
void parse_backslash(std::string& line, size_t& i, std::string& token);

/*handles single and double quotes; prompts for more input if quotes are
mismatched*/
void parse_quotes(std::string& line,
		std::vector< std::vector<std::string> >& parsed,
		size_t& i,
		std::string& token);

//handles the ; connector
int parse_connector1(const std::string& line,
					 std::vector< std::vector<std::string> >& parsed,
					 size_t& i, size_t& j,
					 std::string& token);

//handles the && and || connectors
int parse_connector2(std::string& line,
					 std::vector< std::vector<std::string> >& parsed,
					 size_t& i, size_t& j,
					 std::string& token);

//prompts for and gets more input
void more_input(std::string& line);

#endif
