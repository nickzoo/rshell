#include <iostream> //std::cout, std::cin
#include <string> //std::string, std::getline
#include <vector> //std::vector
#include <stdlib.h> //exit, size_t
#include "parse.h"

//each connector is the first argument of the command it precedes
int parse(std::string& line,
		std::vector< std::vector<std::string> >& parsed) {
	parsed.push_back(std::vector<std::string>());
	size_t i = 0;
	size_t j = 0;
	while (i < line.size()) {
		while (i < line.size() && line[i] <= 32) ++i;
		if (line[i] == '#')
			break;
		std::string token;
		while (i < line.size() && line[i] > 32) {
			if (line[i] == '\\') {
				parse_backslash(line, i, token);
				continue;
			}
			if (line[i] == '\'' || line[i] == '"') {
				parse_quotes(line, parsed, i, token);
				continue;
			}
			else if (line[i] == ';') {
				int error = parse_connector1(line, parsed, i, j, token);
				if (error) return error; //e.g. ;; causes error
				break;
			}
			else if (i < line.size() - 1 &&
					((line[i] == '&' && line[i+1] == '&') ||
					(line[i] == '|' && line[i+1] == '|'))) {
				int error = parse_connector2(line, parsed, i, j, token);
				if (error) return error; //e.g. ;&& causes error
				break;
			}
			token += line[i++];
		}
		parsed[j].push_back(token);
	}
	return 0;
}

void parse_backslash(std::string& line,
					size_t& i,
					std::string& token) {
	if (i+1 < line.size()) {
		token += line[i+1];
		i += 2;
	}
	else {
		more_input(line);
		++i; //skip newline character
		while (i < line.size() && line[i] <= 32) ++i;
	}
}

void parse_quotes(std::string& line,
				 std::vector< std::vector<std::string> >& parsed,
				 size_t& i,
				 std::string& token) {
	char delimiter = line[i];
	++i;
LOOP:
	while (i < line.size() && line[i] != delimiter)
		token += line[i++];
	if (i == line.size()) { //mismatched quotes
		more_input(line);
		goto LOOP;
	}
	++i; //skip quote character
}

int parse_connector1(const std::string& line,
					 std::vector< std::vector<std::string> >& parsed,
					 size_t& i,
					 size_t& j,
					 std::string& token) {
	if (token.size() == 0) {
		if (parsed[j].size() == 0 ||
				(j > 0 && parsed[j].size() == 1)) {
			std::cerr << "syntax error: token '"
				<< line[i] << '\'' << std::endl;
			return 1;
		}
		token += line[i++];
		parsed.push_back(std::vector<std::string>());
		++j;
	}
	else {
		parsed[j].push_back(token);
		token =  "";
		token += line[i++];
		parsed.push_back(std::vector<std::string>());
		++j;
	}
	return 0;
}

int parse_connector2(std::string& line,
					 std::vector< std::vector<std::string> >& parsed,
					 size_t& i,
					 size_t& j,
					 std::string& token) {
	if (token.size() == 0) {
		if (parsed[j].size() == 0 ||
				(j > 0 && parsed[j].size() == 1)) {
			std::cerr << "syntax error: token '" << line[i]
				 << line[i+1] << '\'' << std::endl;
			return 1;
		}
		token += line[i++];
		token += line[i++];
		parsed.push_back(std::vector<std::string>());
		++j;
	}
	else {
		parsed[j].push_back(token);
		token = "";
		token += line[i++];
		token += line[i++];
		parsed.push_back(std::vector<std::string>());
		++j;
	}
	while (i == line.size()) {
		more_input(line);
		++i;
	}
	return 0;
}

void more_input(std::string& line) {
	std::cout << "> ";
	std::string next_line;
	getline(std::cin, next_line);
	line += "\n" + next_line;
}
