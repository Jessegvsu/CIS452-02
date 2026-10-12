
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#define WRITE 1
#define READ 0

void childRun(int pipeIn, int pipeOut, char* args[]) {

	pid_t pid = fork();
	if (pid == 0) {
		if (pipeIn != STDIN_FILENO) { //if in param is not stdin
			dup2(pipeIn, STDIN_FILENO); //make stdin point to in pipe fd.
			close(pipeIn); //close in fd as it is not needed since stdin points to the same pipe input.
		}
		if (pipeOut != STDOUT_FILENO) {// out param is not stdout
			dup2(pipeOut, STDOUT_FILENO); //makestdout point to out pipe
			close(pipeOut); //close out fd as it is not needed since stdout points to the same pipe.
		}

		//printf("child %d's in == %d and out == %d", getpid(), pipeIn, pipeOut);

		execvp(args[0], args);
		perror("execvp failed");
		exit(1);
	}
}

int main() {
	bool isRunning = true;
	char *argv[100];


	while (isRunning) {
		char input[100];
		int pCount = 0;
		int pIndex[100];
		printf("Type in command: ");
		fgets(input, sizeof(input), stdin);
		if (strcmp(input, "quit\n") == 0) {
			isRunning = false;
			break;
		}

		//printf("Attempting to run: %s", input);

		//parse command with strtok()
		char *tok;
		tok = strtok(input, " \n");
		int i = 0;
		int pi = 1;
		pIndex[0] = 0;
		//printf("WHAT IS IN INPUT\n");
		while (tok != NULL) {
			//printf("ARG[%d] = [%s]\n",i, tok);
			if (*tok == '|') {
				argv[i] = NULL;
				pCount++;
				pIndex[pi] = i + 1;
				pi++;
			} else {
				argv[i] = tok;
			}
			tok = strtok(NULL, " \n");
			i++;
		}
		argv[i] = NULL;

		//printf("LAST ARG = [%s]\n", argv[i]);
		//printf("Pipe Count  = [%d]\n", pCount);
		//printf("WHAT IS IN ARGV[]\n");
		//for (int j = 0; j < argCount; j ++) {
			//printf("Actual argv[%d] = [%s]\n", j, argv[j]);
		//
		//printf("Entering pipe/child handler body\n");

		//create pCount amount of pipes
		int pipeIn = STDIN_FILENO;
		for (int i = 0; i <= pCount; i++) {
			int pipeOut = STDOUT_FILENO;
			int fd[2];
			if (i < pCount) {
				pipe(fd);
				pipeOut = fd[WRITE];
			}

			childRun(pipeIn, pipeOut, &argv[pIndex[i]]);

			if (pipeIn != STDIN_FILENO) {
				close(pipeIn);
			}
			if (i < pCount) {
				close(fd[WRITE]);
				pipeIn = fd[READ];
			}

		}
		int status;
		for (int i = 0; i <= pCount; i++) {
			wait(&status);
			//pid_t who = wait(&status);
			//printf("Child %d exited with status is %d\n", who, WEXITSTATUS(status));
		}
	}
	return 0;
}
