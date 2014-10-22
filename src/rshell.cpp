#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h> //perror
#include <stdlib.h> //NULL, exit
#include <string.h> //memcpy, strcmp
#include <sys/wait.h> //waitpid
#include <unistd.h> //fork, execvp
using namespace std;

#define MAX_LENGTH 256 //for login and host name
#define CONNECTOR_ERROR_MESSAGE \
	"syntax error: connector must be preceded by a command"
#define QUOTES_ERROR_MESSAGE \
	"syntax error: unmatched quotes"

//separate special characters with whitespace for tokenization
int preprocess(string& line, vector<string>& quoted) {
	string processed = "";
	for (size_t i = 0; i < line.size(); ++i) {
		if (line[i] == '#')
			processed += "# ";
		else if (line[i] == '"' || line[i] == '\'') {
			if (i == line.size() - 1)
				return 2;
			char delimiter = line[i++];
			string token;
			while (i < line.size() && line[i] != delimiter) {
				token += line[i++];
			}
			if (i == line.size() && line[i-1] != delimiter)
				return 2;
			quoted.push_back(token);
			processed += " ;; ";
		}
		else if (line[i] == ';')
			processed += " ; ";
		else if (line[i] == '&' && i+1 < line.size() && line[i+1] == '&') {
			processed += " && ";
			++i;
		}
		else if (line[i] == '|' && i+1 < line.size() && line[i+1] == '|') {
			processed += " || ";
			++i;
		}
		else
			processed += line[i];
	}
	line = processed;
	return 0;
}

//raw input is parsed into an vector of commands
int parse(string& line, vector< vector<string> >& cmd) {
	int q = 0;
	vector<string> quoted;
	int error = preprocess(line, quoted);
	if (error)
		return 2;
	cmd.push_back(vector<string>());
	stringstream ss(line);
	string token;
	int c = 0;
	while (ss >> token) {
		if (token == "#")
			break;
		else if (token == ";;") {
			token = quoted[q++];
		}
		else if (token == ";" || token == "||" || token == "&&") {
			if (cmd[c].empty() || (c > 0 && cmd[c].size() == 1))
				return 1;
			cmd.push_back(vector<string>());
			++c;
		}
		cmd[c].push_back(token);
	}
	return 0;
}

//converts vector of vector of strings to vector of array of cstrings
vector<char**> c_compatible(const vector< vector<string> >& cmd) {
	vector<char**> c_cmd;
	for (size_t i = 0; i < cmd.size(); ++i) {
		char **command = new char*[cmd[i].size()+1];
		size_t j;
		for (j = 0; j < cmd[i].size(); ++j) {
			command[j] = new char[cmd[i][j].size()+1];
			memcpy(command[j], cmd[i][j].c_str(), cmd[i][j].size()+1);
		}
		command[j] = NULL;
		c_cmd.push_back(command);
	}
	return c_cmd;
}

//deletes c compatible array of cstrings
void c_delete(const vector< vector<string> >& cmd, vector<char**>& c_cmd) {
	for (size_t i = 0; i < cmd.size(); ++i) {
		for (size_t j = 0; j <= cmd[i].size(); ++j) {
			delete[] c_cmd[i][j];
		}
		delete[] c_cmd[i];
	}
}

//execute commands using fork, execvp, and waitpid
void execute(const vector< vector<string> >& cmd) {
	vector<char**> c_cmd = c_compatible(cmd);
	int status;
	for (size_t i = 0; i < c_cmd.size(); ++i) {
		int pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			if (i == 0) {
				execvp(c_cmd[i][0], c_cmd[i]);
				perror("execvp");
				exit(1);
			}
			else {
				if (c_cmd[i][1] == NULL)
					exit(0);
				if (strcmp(c_cmd[i][0], "||") == 0 && status == 0)
					exit(0);
				if (strcmp(c_cmd[i][0], "&&") == 0 && status != 0)
					exit(0);
				execvp(c_cmd[i][1], c_cmd[i]+1);
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
	c_delete(cmd, c_cmd);
}

//prompt for input, get input, parse, execute, repeat
int main() {
	char login[MAX_LENGTH];
	char hostname[MAX_LENGTH];
	while (1) {
		if (getlogin_r(login, MAX_LENGTH) != 0) {
			perror("getlogin_r");
			return -1;
		}
		if (gethostname(hostname, MAX_LENGTH) != 0) {
			perror("gethostname");
			return -1;
		}
		cout << login << "@" << hostname << "$ ";
		string line;
		getline(cin, line);
		if (line == "exit")
			break;
		vector< vector<string> > cmd;
		int error = parse(line, cmd);
		if (error) {
			if (error == 1)
				cerr << CONNECTOR_ERROR_MESSAGE <<  endl;
			else if (error == 2)
				cerr << QUOTES_ERROR_MESSAGE << endl;
			else
				cout << "unknown error" << endl;
			continue;
		}
		execute(cmd);
	}
	return 0;
}
