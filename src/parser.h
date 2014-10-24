#ifndef PARSER_H
#define PARSER_H

int parse(std::string& line,
		std::vector< std::vector<std::string> >& parsed);

void parse_backslash(std::string& line, size_t& i, std::string& token);

void parse_quotes(std::string& line,
		std::vector< std::vector<std::string> >& parsed,
		size_t& i,
		std::string& token);

int parse_connector1(const std::string& line,
					 std::vector< std::vector<std::string> >& parsed,
					 size_t& i, size_t& j,
					 std::string& token);

int parse_connector2(std::string& line,
					 std::vector< std::vector<std::string> >& parsed,
					 size_t& i, size_t& j,
					 std::string& token);

void more_input(std::string& line);

#endif
