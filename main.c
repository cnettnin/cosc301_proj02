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
			//printf("RWS(%s)\n",finishArray[i]);
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
		//printf("in mode check\n");
		if(commandList[i][1] == NULL){
			//printf("Showing current mode\n");
			modeprint(seq);
		}
		else if(strcmp(commandList[i][1],"s") == 0|| strcmp(commandList[i][1],"sequential") == 0|| strcmp(commandList[i][1],"seq") == 0){
			//printf("change nextmode to 1\n");
			nextmode = 1;
		}
		else if(strcmp(commandList[i][1],"p") == 0|| strcmp(commandList[i][1],"parallel") == 0|| strcmp(commandList[i][1],"par") == 0){
			//printf("change nextmode to 0\n");
			nextmode = 0;
		}
		else{
			//printf("%s not a valid mode. Choose parallel or sequential\n",commandList[i][1]);
			modeprint(seq);
		}
		return nextmode;
}
int main(int argc, char **argv) {
	int exit = 0; //tracks exit
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
				//printf("Exit changed to 1\n");
				exit = 1;
			}
			commandList[commandCount] = tokenify(seperatedCommands[commandCount]," ");
			commandCount++;
			
		}

		int i = 0;
		pid_t pid;
		if(seq == 1){ //sequential mode
			while(i < commandCount){
				//printf("commandList[i][0] = %s\n",commandList[i][0]);
				//printf("commandList[i][1] = %s\n",commandList[i][1]);
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
						if (execv(commandList[i][0], commandList[i]) < 0 && !strcmp(commandList[i][0],"exit") && !strcmp(commandList[i][0],"mode")) {
    	   					fprintf(stderr, "execv failed: %s\n", strerror(errno));
  						}
						_exit(EXIT_FAILURE);
					}
				}
			i++;
			}
		}
		else{ //parallel mode
			int n = commandCount;
			printf("commandCount:%d n:%d\n",commandCount,n);
			while(i < commandCount){
				printf("while loop\n");
				if(strncmp(commandList[i][0],"mode",4) == 0){
					nextmode = modeHandle(seq,nextmode,commandList,i);
				}
				//else{
					//pid_t pids[n];
					for(int j = 0; j < n; j++){
						printf("for loop\n");
						switch(fork()){
							case 0:
								if (execv(commandList[i][0], commandList[i]) < 0 && !strcmp(commandList[i][0],"exit") && !strcmp(commandList[i][0],"mode")) {
    	   							fprintf(stderr, "execv failed: %s\n", strerror(errno));
  								}
								_exit(EXIT_SUCCESS);
							case -1:
								perror("fork");
								_exit(EXIT_FAILURE);
							default:
								wait(NULL);
						}
					}
					//if((pids[j] = fork()) < 0){
					//	perror("fork");
					//	abort();
					//}
						
						//_exit(EXIT_FAILURE);
					//int status;
					//pid_t pid;
					//while(n > 0){
						//pid = wait(&status);
					//	n--;
					//}
				//}
			i++;
			}
		}

		//printf("i = %d\n",i);
		if(exit == 1 && i == commandCount){
			return 0;
		}
		/*
		for(int j = 0; j < commandCount; j++){ //free everything
			free_tokens(commandList[j]);
		}
		free(commandList);
		*/
	}
	return 0; //test: /bin/ls -t ; exit ; /bin/pwd
}
