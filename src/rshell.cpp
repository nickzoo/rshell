#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <dirent.h>		// dirent, opendir, readdir, closedir
#include <errno.h>		// errno
#include <fcntl.h>		// O_RDONLY, O_WRONLY, ...
#include <stdio.h>		// perror
#include <stdlib.h>		// exit, NULL, getenv
#include <string.h>		// strcpy, strtok
#include <sys/wait.h>	// wait
#include <unistd.h>		// getlogin_r, gethostname, dup, pipe
using namespace std;

#define MAXLENGTH 256

struct Command {
	vector<string> argv;
	string connector;
	string inputfile;
	string outputfile;
	int append;
	Command(): append(O_TRUNC) { }
};

vector<Command> cmds;
map<string, string> pmap;
int ischild = 0;

void interrupthandler(int n);
char* getrelative(char *absolute);
int parse(string& line);
int execute();

int main() {
	// get directories in PATH
	char *path = getenv("PATH");
	if (path == NULL) {
		cerr << "PATH not found" << endl; return 1;
	}
	vector<char*> directories;
	char *token = strtok(path, ":");
	while (token) {
		directories.push_back(token);
		token = strtok(NULL, ":");
	}

	// populate pmap (path map)
	for (size_t i = 0; i < directories.size(); ++i) {
		DIR *dirptr = opendir(directories[i]);
		if (dirptr == NULL) {
			perror("opendir"); return 1;
		}
		dirent *entry;
		while ((entry = readdir(dirptr)))
			pmap[entry->d_name] = directories[i];
		if (errno) {
			perror("readdir"); return 1;
		}
		if (closedir(dirptr) == -1) {
			perror("closedir"); return 1;
		}
	}

	// initialize signal handler
	if (signal(SIGINT, interrupthandler) == SIG_ERR) {
		perror("signal"); return 1;
	}

	char currentdir[MAXLENGTH];
	// prompt and get input
	while (1) {
		if (getcwd(currentdir, sizeof(currentdir)) == NULL) {
			perror("getcwd"); return 1;
		}
		cout << currentdir << "$ ";
		string line; getline(cin, line);

		// parse returns 1 on failure; execute returns 1 to exit
		if (parse(line)) continue;
		if (execute()) break;
		cmds.clear();
	}
	return 0;
}

void interrupthandler(int n) {
	if (ischild) {
		cout << endl;
		exit(0);
	}
	else {
		cmds.clear();
	}
}

int parse_token(string& line, size_t& i, size_t& j, string& token);
void parse_escape(string& line, size_t& i, string& token);
void parse_quotes(string& line, size_t& i, string& token);
int parse_ioredirect(string& line, size_t& i, size_t& j);
void moreinput(string& line);

int parse(string& line) {
	// initialize
	cmds.push_back(Command());
	size_t i = 0, j = 0;

	// parse_token returns 0 on success, -1 on comment, 1 on error
	while (i < line.size()) {
		string token;
		int msg = parse_token(line, i, j, token);
		if (msg == -1) break;
		if (msg ==  1) return 1;
		if (!token.empty()) cmds[j].argv.push_back(token);
	}

	// prepend paths
	for (size_t i = 0; i < cmds.size(); ++i) {
		string filename = cmds[i].argv[0];
		if (filename == "cd") continue;
		if (!pmap[filename].empty())
			cmds[i].argv[0] = pmap[filename] + '/' + filename;
	}
	return 0;
}

int parse_token(string& line, size_t& i, size_t& j, string& token) {
	// ignore whitespace and check for comment
	while (i < line.size() && line[i] <= 32) ++i;
	if (line[i] == '#') return -1;

	// if the token is a redirect or connector, it's a special case
	if (line[i] == '<' || line[i] == '>') {
		if (parse_ioredirect(line, i, j)) return 1;
		return 0;
	}
	if (line[i] == ';') {
		if (cmds[j].argv.empty()) {
			cerr << "syntax error: token ';'" << endl;
			return 1;
		}
		cmds.push_back(Command());
		cmds[++j].connector = ";";
		++i;
		return 0;
	}
	if (i < line.size()-1 && ((line[i] == '&' && line[i+1] == '&') ||
	line[i] == '|' && line[i+1] == '|')) {
		string connector = line[i] == '&' ? "&&" : "||";
		if (cmds[j].argv.empty()) {
			cerr << "syntax error: token '" << connector << "'" << endl;
			return 1;
		}
		cmds.push_back(Command());
		cmds[++j].connector = connector;
		i += 2;
		return 0;
	}
	if (line[i] == '|') {
		if (cmds[j].argv.empty()) {
			cerr << "syntax error: token '|'" << endl;
			return 1;
		}
		cmds.push_back(Command());
		cmds[++j].connector = "|";
		++i;
		return 0;
	}

	// iterate until end of line, whitespace, redirect, or connector
	while (i < line.size() && line[i] > 32) {
		if (line[i] == '\\') {
			parse_escape(line, i, token);
			continue;
		}
		if (line[i] == '\'' || line[i] == '"') {
			parse_quotes(line, i, token);
			continue;
		}
		if (line[i] == '<' || line[i] == '>' ||
		line[i] == '|' || line[i] == ';' || (i < line.size()-1 &&
		((line[i] == '&' && line[i+1] == '&') ||
		  line[i] == '|' && line[i+1] == '|')))
			break;
		token += line[i++];
	}
	return 0;
}

void parse_escape(string& line, size_t& i, string& token) {
	// if end of line, get more input, else escape a character
	if (++i < line.size()) {
		token += line[i++];
	}
	else {
		moreinput(line);
		++i;
	}
}

void parse_quotes(string& line, size_t& i, string& token) {
	// escape everything inside quotes
	char delimiter = line[i++];
SEEKDELIMITER:
	while (i < line.size() && line[i] != delimiter) token += line[i++];

	// if quotes mismatched, get more input
	if (i == line.size()) {
		moreinput(line);
		goto SEEKDELIMITER;
	}
	++i;
}

int parse_ioredirect(string& line, size_t& i, size_t& j) {
	// check for ambiguous io redirect and append token ">>"
	char c = line[i++];
	if ((c == '<' && cmds[j].inputfile != "") ||
		(c == '>' && cmds[j].outputfile != "")) {
		cerr << "error: ambiguos io redirection" << endl;
		return 1;
	}
	if (c == '>' && i < line.size() && line[i] == '>') {
		cmds[j].append = O_APPEND;
		++i;
	}

	// parse file for io
	string token;
	if (parse_token(line, i, j, token)) return 1;
	if (token.empty()) {
		cerr << "syntax error: no file specified" << endl;
		return 1;
	}
	if (c == '<') cmds[j].inputfile  = token;
	else		  cmds[j].outputfile = token;
	return 0;
}

void moreinput(string& line) {
	// get another line of input
	cout << "> ";
	string nextline;
	getline(cin, nextline);
	line += "\n" + nextline;
}

char** c_compatible(vector<string> argv);
void c_delete(int argc, char **argv);

int execute() {
	// initialize
	int previousfd[2];
	int currentfd[2];
	if (pipe(currentfd) == -1) {
		perror("pipe"); exit(1);
	}
	int status;

	for (size_t i = 0; i < cmds.size(); ++i) {
		// check for empty argv and exit
		if (cmds[i].argv.empty()) return 0;
		if (cmds[i].argv[0] == "exit") {
			return 1;
		}

		// handle cd
		if (cmds[i].argv[0] == "cd") {
			if (cmds[i].argv.size() < 2) {
				if (chdir(getenv("HOME")) == -1) {
					perror("chdir"); return 0;
				} continue;
			}
			if (chdir(cmds[i].argv[1].c_str()) == -1) { 
				perror("chdir"); return 0;
			}
			continue;
		}

		// handle being piped to (parent process)
		if (cmds[i].connector == "|") {
			previousfd[0] = currentfd[0];
			previousfd[1] = currentfd[1];
			if (pipe(currentfd) == -1) {
				perror("pipe"); exit(1);
			}
		}

		// convert string vector to cstring array and fork
		char **c_argv = c_compatible(cmds[i].argv);
		int pid = fork();
		if (pid == -1) {
			perror("fork"); exit(1);
		}
		else if (pid == 0) {
			// exit on SIGINT
			ischild = 1;

			// evaluate conditionals
			if ((cmds[i].connector == "||" && status == 0) ||
				(cmds[i].connector == "&&" && status != 0))
				exit(0);

			// handle being piped to (child process)
			if (cmds[i].connector == "|") {
				if (dup2(previousfd[0], STDIN_FILENO) == -1) {
					perror("dup2"); exit(1);
				}
				if (close(previousfd[1]) == -1) {
					perror("close"); exit(1);
				}
			}

			// handle piping to another process
			if (i+1 < cmds.size() && cmds[i+1].connector == "|") {
				if (dup2(currentfd[1], STDOUT_FILENO) == -1) {
					perror("dup2"); exit(1);
				}
				if (close(currentfd[0]) == -1) {
					perror("close"); exit(1);
				}
			}

			// handle input file
			if (!cmds[i].inputfile.empty()) {
				int fdin = open(cmds[i].inputfile.c_str(), O_RDONLY);
				if (fdin == -1) {
					perror("open"); exit(1);
				}
				if (dup2(fdin, STDIN_FILENO) == -1) {
					perror("dup2"); exit(1);
				}
			}

			// handle output file
			if (!cmds[i].outputfile.empty()) {
				int fdout = open(cmds[i].outputfile.c_str(),
					cmds[i].append | O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
				if (fdout == -1) {
					perror("open"); exit(1);
				}
				if (dup2(fdout, STDOUT_FILENO) == -1) {
					perror("dup2"); exit(1);
				}
			}

			// execute
			execv(c_argv[0], c_argv);
			perror("execvp"); exit(1);
		}
		else {
			// close parent pipe if being piped to and wait
			if (cmds[i].connector == "|") {
				if (close(previousfd[0]) == -1 ||
					close(previousfd[1]) == -1) {
					perror("close"); exit(1);
				}
			}
			if (wait(&status) == -1) {
				perror("wait"); exit(1);
			}
		}
		c_delete(cmds[i].argv.size(), c_argv);
	}
	return 0;
}

// conversion is necessary for c exec functions
char** c_compatible(vector<string> argv) {
	char **c_argv = new char*[argv.size()+1];
	size_t i; for (i = 0; i < argv.size(); ++i) {
		c_argv[i] = new char[argv[i].size()+1];
		strcpy(c_argv[i], argv[i].c_str());
	}
	c_argv[i] = NULL;
	return c_argv;
}

// prevent memory leak
void c_delete(int argc, char **c_argv) {
	for (size_t i = 0; i < argc; ++i) delete[] c_argv[i];
}
