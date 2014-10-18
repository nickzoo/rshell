#include <stdio.h> //printf, fgets, stdin
#include <stdlib.h> //NULL, exit
#include <string.h> //strcpy, strtok
#include <sys/wait.h> //waitpid
#include <unistd.h> //fork, execvp

#define DELIMITERS " \t\n" //for strtok
#define MAX_NAME 16 //login and hostname
#define MAX_CHARS 1024 //line of input
#define MAX_CMDS 16 //commands delimited by connectors
#define MAX_ARGS 64 //arguments for each command
#define PREP_EXCEEDED 1
#define CMDS_EXCEEDED 2
#define ARGS_EXCEEDED 3
#define CONNECTOR_ERROR 4
#define PREP_EXCEEDED_MESSAGE \
	"overflow: too many characters due to preprocessing\n"
#define CMDS_EXCEEDED_MESSAGE \
	"overflow: too many commands\n"
#define ARGS_EXCEEDED_MESSAGE \
	"overflow: too many arguments\n"
#define CONNECTOR_ERROR_MESSAGE \
	"syntax error: connector must be preceded by a command\n"
#define LINE_EXCEEDED_MESSAGE \
	"overflow: too many characters\n"

//separate special characters by whitespace for tokenization
int preprocess(char line[]) {
	char processed[MAX_CHARS + 1];
	int i = 0, j = 0;
	while (line[i] != '\0') {
		if (line[i] == ';' || line[i] == '#') {
			processed[j++] = ' ';
			processed[j++] = line[i++];
			processed[j++] = ' ';
		}
		else if ((line[i] == '&' && line[i+1] == '&')
				|| (line[i] == '|' && line[i+1] == '|')) {
			processed[j++] = ' ';
			processed[j++] = line[i++];
			processed[j++] = line[i++];
			processed[j++] = ' ';
		}
		else
			processed[j++] = line[i++];
		if (j > MAX_CHARS-1)
			return PREP_EXCEEDED;
	}
	processed[j] = '\0';
	strcpy(line, processed);
	return 0;
}

//raw input is parsed into an array of commands
int parse(char line[], char *cmds[][MAX_ARGS]) {
	int error = preprocess(line);
	if (error)
		return PREP_EXCEEDED;
	int c = 0, a = 0; //c for command index, a for argument index
	char *token = strtok(line, DELIMITERS);
	while (token != NULL) {
		if (strcmp(token, "#") == 0)
			break;
		if (strcmp(token, ";") == 0 || strcmp(token, "||") == 0
				|| strcmp(token, "&&") == 0) {
			if (a == 0)
				return CONNECTOR_ERROR;
			cmds[c++][a] = NULL;
			if (c >= MAX_CMDS-1)
				return CMDS_EXCEEDED;
			a = 0;
		}
		cmds[c][a++] = token;
		if (a > MAX_ARGS-1)
			return ARGS_EXCEEDED;
		token = strtok(NULL, DELIMITERS);
	}
	cmds[c][a] = NULL;
	cmds[c+1][0] = NULL;
	return 0;
}

//execute commands using fork, execvp, and waitpid
void execute(char *cmds[][MAX_ARGS]) {
	int status;
	for (int c = 0; cmds[c][0]; ++c) {
		int pid = fork();
		if (pid < 0) {
			perror("fork");
			exit(1);
		}
		else if (pid == 0) {
			if (c == 0) {
				execvp(cmds[c][0], cmds[c]);
				perror(cmds[c][0]);
				exit(1);
			}
			else {
				if (strcmp(cmds[c][0], "||") == 0 && status == 0)
					exit(0);
				if (strcmp(cmds[c][0], "&&") == 0 && status != 0)
					exit(0);
				execvp(cmds[c][1], cmds[c]+1);
				perror(cmds[c][0]);
				exit(1);
			}
		}
		else {
			if (waitpid(pid, &status, 0) < 0)
				perror("waitpid");
		}
	}
}

//remove leftovers from stdin after line overflows
void flush() {
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
}

//prompt for input, get input, parse, execute, repeat
int main() {
	const char *errors[] = {PREP_EXCEEDED_MESSAGE, CMDS_EXCEEDED_MESSAGE,
		ARGS_EXCEEDED_MESSAGE, CONNECTOR_ERROR_MESSAGE};
	char line[MAX_CHARS];
	char *cmds[MAX_CMDS][MAX_ARGS];
	while (1) {
		char login[MAX_NAME];
		getlogin_r(login, MAX_NAME);
		char hostname[MAX_NAME];
		gethostname(hostname, MAX_NAME);
		printf("%s@%s$ ", login, hostname);
		fgets(line, sizeof(line), stdin);
		if (strlen(line) >= MAX_CHARS-1 && line[strlen(line)-1] != '\n') {
			printf(LINE_EXCEEDED_MESSAGE);
			flush();
			continue;
		}
		int error = parse(line, cmds);
		if (error) {
			printf("%s", errors[error-1]);
			continue;
		}
		if (strcmp(line, "exit") == 0)
			exit(0);
		execute(cmds);
	}
	return 0;
}
