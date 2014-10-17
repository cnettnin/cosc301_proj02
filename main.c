//Logan Keig & Cory Nettnin
//COSC 301 Project 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>

//adapted from lab01 removewhitespace()
char* removeleadingwhitespace(char s[]) {
	while(isspace(*s)){
		s++;
	}
	return s;
}

//tokenify and associated functions from cosc301_lab02
char** tokenify(const char *s,char *seperator) {
	char *tempstr = strdup(s);
	char *token = strtok(tempstr,seperator);
	int count = 0;
	while(token != NULL){ //counts up number of tokens
		count++;
		token = strtok(NULL,seperator);
	}
	free(tempstr);

	tempstr = strdup(s); //restarts tempstr
	token = strtok(tempstr,seperator);
	char** finishArray = malloc(sizeof(char*)*(count+1)); //finishArray on heap
	for(int i = 0; i < count; i++){
		if(token != NULL){
			finishArray[i] = removeleadingwhitespace(strdup(token));
		}
		token = strtok(NULL,seperator);
	}
	finishArray[count] = NULL;
	free(tempstr);

	return finishArray;
}

void print_tokens(char *tokens[]) {
    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: %s\n", i+1, tokens[i]);
        i++;
    }
}

void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

void modeprint(int seq){
	if(seq==1){
		printf("Current Mode: Sequential\n");
	}
	else{
		printf("Current Mode: Parallel\n");
	}
}

int modeHandle(int seq, int nextmode, char*** commandList,int i) {
		if(commandList[i][1] == NULL){
			modeprint(seq);
		}
		else if(strcmp(commandList[i][1],"s") == 0|| strcmp(commandList[i][1],"sequential") == 0|| strcmp(commandList[i][1],"seq") == 0){
			nextmode = 1;
		}
		else if(strcmp(commandList[i][1],"p") == 0|| strcmp(commandList[i][1],"parallel") == 0|| strcmp(commandList[i][1],"par") == 0){
			nextmode = 0;
		}
		else{
			modeprint(seq);
		}
		return nextmode;
}
int main(int argc, char **argv) {
	int completed = 0; //tracks whether or not to exit
	int seq = 1; //tracks mode (seq = sequential)
	int nextmode = 1; //changes mode for next line (if changed)
	while(1) {
		seq = nextmode; //change mode

		printf("%s", "MyShell>> ");
		fflush(stdout);
		char buffer[1024];
		fgets(buffer, 1024, stdin);
		for(int i = 0;i < strlen(buffer); i++){
			if(buffer[i] == '#'){
				buffer[i] = '\0';
				break;
			}
			if(buffer[i] == '\n'){
				buffer[i] = ' ';
			}
		}

		char** seperatedCommands = tokenify(buffer,";");
		
		char** commandList[1024];
		int commandCount = 0;
		while(seperatedCommands[commandCount] != NULL){
			if(strncmp(seperatedCommands[commandCount],"exit",4) == 0){ //if "exit" is a token, change variable to 1
				completed = 1;
			}
			commandList[commandCount] = tokenify(seperatedCommands[commandCount]," ");
			commandCount++;
			
		}
		int i = 0;
		if(seq == 1){ //sequential mode
			while(i < commandCount){
				if(strncmp(commandList[i][0],"mode",4) == 0){
					nextmode = modeHandle(seq,nextmode,commandList,i);
				}
				else{
					pid_t pid = fork(); //refer to the child with pid
					if(pid > 0){
						wait(NULL);
					}
					else{
						//printf("%s\n",commandList[i][0]);
						if ((execv(commandList[i][0], commandList[i])) < 0 && (strncmp(commandList[i][0],"exit", 4) != 0) && (strncmp(commandList[i][0],"mode",4) != 0)) {
    	   						fprintf(stderr, "execv failed: %s\n", strerror(errno));
  						}
						exit(EXIT_FAILURE);
					}
				}
			i++;
			}
		}
		
		/* 
		Parallel mode
		Code assistance gained from http://stackoverflow.com/questions/2708477/fork-and-wait-with-two-child-processes
		*/
		else{
			pid_t child_pid, wpid;
			int status = 0;
			if(strncmp(commandList[i][0],"mode",4) == 0){
				nextmode = modeHandle(seq,nextmode,commandList,i);
			}
			for (i = 0; i < commandCount; i++) {
				if ((child_pid = fork()) == 0) {
					if ((execv(commandList[i][0], commandList[i])) < 0 && (strncmp(commandList[i][0],"exit", 4) != 0) && (strncmp(commandList[i][0],"mode",4) != 0)) {
    	   					fprintf(stderr, "execv failed: %s\n", strerror(errno));
  					}
				}
			}
			while ((wpid = wait(&status)) > 0) {
			//this doesn't actually do anything, just allows all children to run in parallel instead of stopping and starting after each one
			}
		}


		if(completed == 1 && i == commandCount){
			return 0;
		}

		for(int j = 0; j < commandCount; j++){ //free everything
			free_tokens(commandList[j]);
		}
		free(commandList);
		free(seperatedCommands);

	}
	return 0; //test: /bin/ls -t ; exit ; /bin/pwd
}
