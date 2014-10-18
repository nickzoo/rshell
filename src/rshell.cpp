#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h> //NULL, exit
#include <string.h> //strcmp
#include <sys/wait.h> //waitpid
#include <unistd.h> //fork, execvp
using namespace std;

#define MAX_LENGTH 256 //for login and host name
#define CONNECTOR_ERROR_MESSAGE \
	"syntax error: connector must be preceded by a command"

//separate special characters with whitespace for tokenization
string preprocess(const string& line) {
	string processed = "";
	for (int i = 0; i < line.size(); ++i) {
		if (line[i] == ';')
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
	return processed;
}

//raw input is parsed into an array of commands
int parse(string& line, vector< vector<string> >& cmd) {
	line = preprocess(line);
	cmd.push_back(vector<string>());
	stringstream ss(line);
	string token;
	int i = 0;
	while (ss >> token) {
		if (token == "#")
			break;
		if (token == ";" || token == "||" || token == "&&") {
			if (cmd[i].empty())
				return 1;
			cmd.push_back(vector<string>());
			++i;
		}
		cmd[i].push_back(token);
	}
	return 0;
}

//converts vector of vector of strings to vector of array of cstrings
vector<char**> c_compatible(const vector< vector<string> >& cmd) {
	vector<char**> c_cmd;
	for (int i = 0; i < cmd.size(); ++i) {
		char **command = new char*[cmd[i].size()+1];
		int j;
		for (j = 0; j < cmd[i].size(); ++j) {
			command[j] = new char[cmd[i][j].size()];
			memcpy(command[j], cmd[i][j].c_str(), cmd[i][j].size()+1);
		}
		command[j] = NULL;
		c_cmd.push_back(command);
	}
	return c_cmd;
}

//deletes c compatible array of cstrings
void c_delete(const vector< vector<string> >& cmd, vector<char**>& c_cmd) {
	for (int i = 0; i < cmd.size(); ++i) {
		for (int j = 0; j <= cmd[i].size(); ++j) {
			delete[] c_cmd[i][j];
		}
		delete c_cmd[i];
	}
}

//execute commands using fork, execvp, and waitpid
void execute(const vector< vector<string> >& cmd) {
	vector<char**> c_cmd = c_compatible(cmd);
	int status;
	for (int i = 0; i < c_cmd.size(); ++i) {
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
			if (waitpid(pid, &status, 0) < 0)
				perror("waitpid");
		}
	}
	c_delete(cmd, c_cmd);
}

//prompt for input, get input, parse, execute, repeat
int main() {
	while (1) {
		char *login = getlogin();
		char hostname[MAX_LENGTH];
		gethostname(hostname, MAX_LENGTH);
		cout << login << "@" << hostname << "$ ";
		string line;
		getline(cin, line);
		if (line == "exit")
			exit(0);
		vector< vector<string> > cmd;
		if (parse(line, cmd)) {
			cout << CONNECTOR_ERROR_MESSAGE <<  endl;
			continue;
		}
		execute(cmd);
	}
	return 0;
}
