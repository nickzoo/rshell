#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h> //perror
#include <stdlib.h> //NULL, exit
#include <string.h> //strcpy, strcmp
#include <sys/wait.h> //waitpid
#include <unistd.h> //fork, execvp
using namespace std;

#define MAX_LENGTH 256

void more_input(string& line) {
	cout << "> ";
	string next_line;
	if (cin.eof())
		exit(1);
	getline(cin, next_line);
	line += "\n" + next_line;
}

void parse_backslash(string& line,
					size_t& i,
					string& token) {
	if (i+1 < line.size()) {
		token += line[i+1];
		i += 2;
	}
	else {
		more_input(line);
		i += 1;
		while (i < line.size() && line[i] <= 32) ++i;
	}
}

void parse_quotes(string& line,
				 vector< vector<string> >& parsed,
				 size_t& i,
				 string& token) {
	char delimiter = line[i];
	++i;
LOOP:
	while (i < line.size() && line[i] != delimiter)
		token += line[i++];
	if (i == line.size()) {
		more_input(line);
		goto LOOP;
	}
	++i;
}

int parse_connector1(const string& line,
					 vector< vector<string> >& parsed,
					 size_t& i,
					 size_t& j,
					 string& token) {
	if (token.size() == 0) {
		if (parsed[j].size() == 0 ||
				(j > 0 && parsed[j].size() == 1)) {
			cerr << "syntax error: token '" << line[i] << '\'' << endl;
			return 1;
		}
		token += line[i++];
		parsed.push_back(vector<string>());
		++j;
	}
	else {
		parsed[j].push_back(token);
		token =  "";
		token += line[i++];
		parsed.push_back(vector<string>());
		++j;
	}
	return 0;
}

int parse_connector2(string& line,
					 vector< vector<string> >& parsed,
					 size_t& i,
					 size_t& j,
					 string& token) {
	if (token.size() == 0) {
		if (parsed[j].size() == 0 ||
				(j > 0 && parsed[j].size() == 1)) {
			cerr << "syntax error: token '" << line[i]
				 << line[i+1] << '\'' << endl;
			return 1;
		}
		token += line[i++];
		token += line[i++];
		parsed.push_back(vector<string>());
		++j;
	}
	else {
		parsed[j].push_back(token);
		token = "";
		token += line[i++];
		token += line[i++];
		parsed.push_back(vector<string>());
		++j;
	}
	while (i == line.size()) {
		more_input(line);
		++i;
	}
	return 0;
}

int parse(string& line, vector< vector<string> >& parsed) {
	parsed.push_back(vector<string>());
	size_t i = 0;
	size_t j = 0;
	while (i < line.size()) {
		while (i < line.size() && line[i] <= 32) ++i;
		if (line[i] == '#')
			break;
		string token = "";
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
				if (error) return error;
				break;
			}
			else if (i < line.size() - 1 &&
					((line[i] == '&' && line[i+1] == '&') ||
					(line[i] == '|' && line[i+1] == '|'))) {
				int error = parse_connector2(line, parsed, i, j, token);
				if (error) return error;
				break;
			}
			token += line[i++];
		}
		parsed[j].push_back(token);
	}
	return 0;
}

vector<char**> c_compatible(const vector< vector<string> >& parsed) {
	vector<char**> c_parsed;
	for (size_t i = 0; i < parsed.size(); ++i) {
		char **argv = new char*[parsed[i].size()+1];
		size_t j;
		for (j = 0; j < parsed[i].size(); ++j) {
			argv[j] = new char[parsed[i][j].size()+1];
			memcpy(argv[j], parsed[i][j].c_str(), parsed[i][j].size()+1);
		}
		argv[j] = NULL;
		c_parsed.push_back(argv);
	}
	return c_parsed;
}

//an array of cstrings is necessary for execvp
void c_delete(const vector< vector<string> >& parsed, vector<char**>& c_parsed) {
	for (size_t i = 0; i < parsed.size(); ++i) {
		for (size_t j = 0; j < parsed[i].size(); ++j) {
			delete[] c_parsed[i][j];
		}
		delete[] c_parsed[i];
	}
}

int execute(const vector< vector<string> >& parsed) {
	vector<char**> c_parsed = c_compatible(parsed);
	int status;
	for (size_t i = 0; i < c_parsed.size(); ++i) {
		if (!c_parsed[i][0]) return 0;
		if (strcmp(c_parsed[i][0], "exit") == 0 ||
				(i > 0 && c_parsed[i][1] &&
				 strcmp(c_parsed[i][1], "exit") == 0))
			return 1;
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
		cout << login << "@" << hostname << "$ ";
		string line;
		if (cin.eof())
			break;
		getline(cin, line);
		vector< vector<string> > parsed;
		int error = parse(line, parsed);
		if (error) {
			continue;
		}
		bool exit_now = execute(parsed);
		if (exit_now) {
			break;
		}
	}
	return 0;
}
