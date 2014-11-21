#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fcntl.h>		//O_RDONLY, O_WRONLY, ...
#include <stdio.h>		//perror
#include <stdlib.h>		//exit, NULL
#include <string.h>		//strcmp, strcpy
#include <sys/wait.h>	//wait
#include <unistd.h>		//getlogin_r, gethostname
#include "rshell.h"
using namespace std;

#define MAX_LENGTH 256
#define APPEND 1
 
vector< vector<string> > commands;
vector<string> connectors;
map<size_t, string> input_files;
map<size_t, string> output_files;
map<size_t, bool> append;

int main() {
	char login[MAX_LENGTH];
	char hostname[MAX_LENGTH];
	while (1) {
		if (getlogin_r(login, MAX_LENGTH) != 0) {
			perror("getlogin_r");
			return 1;
		}
		if (gethostname(hostname, MAX_LENGTH) != 0) {
			perror("gethostname");
			return 1;
		}
		cout << login << '@' << hostname << "$ ";
		string line;
		getline(cin, line);
		if (parse(line)) continue;
		if (execute()) break;
	}
	return 0;
}

int parse(string& line) {
	commands.clear(); connectors.clear();
	input_files.clear(); output_files.clear(); append.clear();
	commands.push_back(vector<string>());
	size_t i = 0;
	size_t j = 0;
	while (i < line.size()) {
		while (i < line.size() && line[i] <= 32) ++i;
		if (line[i] == '#') break;
		string token;
		while (i < line.size() && line[i] > 32) {
			if (line[i] == '\\') {
				parse_escape(line, ++i, token);
				continue;
			}
			if (line[i] == '\'' || line[i] == '"') {
				parse_quotes(line, ++i, token);
				continue;
			}
			if (line[i] == '<' || line[i] == '>') {
				int error = parse_ioredirect(line, ++i, j, token);
				if (error) return error;
				continue;
			}
			if (line[i] == ';') {
				int error = parse_end(++i, j, token);
				if (error) return error;
				break;
			}
			if (i < line.size() - 1 &&
				((line[i] == '&' && line[i+1] == '&') ||
				 (line[i] == '|' && line[i+1] == '|'))) {
				int error = parse_conditional(line, ++i, j, token);
				if (error) return error;
				break;
			}
			if (line[i] == '|') {
				int error = parse_pipe(line, ++i, j, token);
				if (error) return error;
				break;
			}
			token += line[i++];
		}
		if (!token.empty()) commands[j].push_back(token);
	}
	return 0;
}

void parse_escape(string& line, size_t& i, string& token) {
	if (i < line.size()) {
		token += line[i++];
	}
	else {
		more_input(line);
		++i;
	}
}

void parse_quotes(string& line, size_t& i, string& token) {
	char delimiter = line[i-1];
	SEEK_DELIMITER:
	while (i < line.size() && line[i] != delimiter) token += line[i++];
	if (i == line.size()) {
		more_input(line);
		goto SEEK_DELIMITER;
	}
	++i;
}

int parse_ioredirect(string& line, size_t& i, size_t& j, string& token) {
	char c = line[i-1];
	if ((c == '<' && input_files[j] != "")||
		(c == '>' && output_files[j] != "")) {
		cerr << "error: ambiguous i/o redirection" << endl;
		return 1;
	}
	if (c == '>' && i < line.size() && line[i] == '>') {
		append[j] = APPEND;
		++i;
	}
	if (!token.empty()) commands[j].push_back(token);
	token = "";
	while (i < line.size() && line[i] <= 32) ++i;
	while (i < line.size() && line[i] > 32 &&
		   line[i] != '<' && line[i] != '>' &&
		   line[i] != ';' && line[i] != '&' && line[i] != '|') {
		token += line[i++];
	}
	if (token == "") {
		cerr << "syntax error: no file specified" << endl;
		return 1;
	}
	if (c == '<')
		input_files[j] = token;
	else
		output_files[j] = token;
	token = "";
	return 0;
}

int parse_end(size_t& i, size_t& j, string& token) {
	if (token.empty() && commands[j].empty()) {
		cerr << "syntax error: token ';'" << endl;
		return 1;
	}
	if (!token.empty()) commands[j].push_back(token);
	token = "";
	connectors.push_back(";");
	commands.push_back(vector<string>());
	++j;
	return 0;
}

int parse_conditional(string& line, size_t& i, size_t& j, string& token) {
	string connector = line[i] == '&' ? "&&" : "||";
	if (token.empty() && commands[j].empty()) {
		cerr << "syntax error: token '" << connector << "'" << endl;
		return 1;
	}
	if (!token.empty()) commands[j].push_back(token);
	token = "";
	connectors.push_back(connector);
	commands.push_back(vector<string>());
	++i; ++j;
	while (i < line.size() && line[i] <= 32) ++i;
	while (i == line.size()) {
		more_input(line);
		while (i < line.size() && line[i] <= 32) ++i;
	}
	return 0;
}

int parse_pipe(string& line, size_t& i, size_t& j, string& token) {
	if (token.empty() && commands[j].empty()) {
		cerr << "syntax error: token |" << endl;
		return 1;
	}
	if (!token.empty()) commands[j].push_back(token);
	token = "";
	commands.push_back(vector<string>());
	connectors.push_back("|");
	++j;
	return 0;
}

void more_input(string& line) {
	cout << "> ";
	string nextline;
	getline(cin, nextline);
	line += "\n" + nextline;
}

int execute() {
	int previous_fd[2];
	int current_fd[2];
	if (pipe(current_fd) == -1) {
		perror("pipe");
		return 1;
	}
	vector<char**> c_commands = c_compatible();
	int status;
	int fd_in = -1;
	int fd_out = -1;
	for (size_t i = 0; i < commands.size(); ++i) {
		if (commands[i].empty()) return 0;
		if (commands[i][0] == "exit") {
			c_delete(c_commands);
			return 1;
		}
		if (i > 0 && connectors[i-1] == "|") {
			previous_fd[0] = current_fd[0];
			previous_fd[1] = current_fd[1];
			pipe(current_fd);
		}
		int pid = fork();
		if (pid == -1) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			if (i > 0 &&
				((connectors[i-1] == "||" && status == 0) ||
				 (connectors[i-1] == "&&" && status != 0))) exit(0);
			if (i > 0 && connectors[i-1] == "|") {
				if (dup2(previous_fd[0], STDIN_FILENO) == -1) {
					perror("dup2");
					exit(1);
				}
				if (close(previous_fd[1]) == -1) {
					perror("close");
					exit(1);
				}
			}
			if (i < connectors.size() && connectors[i] == "|") {
				if (dup2(current_fd[1], STDOUT_FILENO) == -1) {
					perror("dup2");
					exit(1);
				}
				if (close(current_fd[0]) == -1) {
					perror("close");
					exit(1);
				}
			}
			if (input_files[i] != "") {
				fd_in = open(input_files[i].c_str(), O_RDONLY);
				if (fd_in == -1) {
					perror("open");
					exit(1);
				}
				if (dup2(fd_in, STDIN_FILENO) == -1) {
					perror("dup2");
					exit(1);
				}
			}
			if (output_files[i] != "") {
				if (append[i])
					fd_out = open(output_files[i].c_str(),
						O_APPEND | O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
				else
					fd_out = open(output_files[i].c_str(),
						O_TRUNC | O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
				if (fd_out == -1) {
					perror("open");
					exit(1);
				}
				if (dup2(fd_out, STDOUT_FILENO) == -1) {
					perror("dup2");
					exit(1);
				}
			}
			execvp(c_commands[i][0], c_commands[i]);
			perror("execvp");
			exit(1);
		}
		else {
			if (i > 0 && connectors[i-1] == "|") {
				if (close(previous_fd[0]) == -1 || close(previous_fd[1])) {
					perror("close");
					exit(1);
				}
			}
			if (wait(&status) == -1) {
				perror("waitpid");
				exit(1);
			}
			if (fd_in != -1) {
				if (close(fd_in) == -1) {
					perror("close");
					exit(1);
				}
			}
			if (fd_out != -1) {
				if (close(fd_out == -1)) {
					perror("close");
					exit(1);
				}
			}
		}
	}
	c_delete(c_commands);
	return 0;
}

vector<char**> c_compatible() {
	vector<char**> c_commands;
	for (size_t i = 0; i < commands.size(); ++i) {
		char **argv = new char*[commands[i].size()+1];
		size_t j;
		for (j = 0; j < commands[i].size(); ++j) {
			argv[j] = new char[commands[i][j].size()+1];
			strcpy(argv[j], commands[i][j].c_str());
		}
		argv[j] = NULL;
		c_commands.push_back(argv);
	}
	return c_commands;
}

void c_delete(vector<char**>& c_commands) {
	for (size_t i = 0; i < commands.size(); ++i) {
		for (size_t j = 0; j < commands[i].size(); ++j)
			delete[] c_commands[i][j];
		delete[] c_commands[i];
	}
}
