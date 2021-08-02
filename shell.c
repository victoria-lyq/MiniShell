#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h> 
#include <errno.h>
#include <string.h>
#include <dirent.h>     //Used for handling directory files

#define BUFFER_SIZE 80


//Globals
char* built_in_commands[] = {"exit", "help", "pwd", "cd", "guessinggame"};
int num_built_in = 5;
char* myCommand;
char** tokens;
char** tokens2;

// Create a signal handler
void sigint_handler(int sig){
    // Ask yourself why is 35 the last parameter here?
    write(1,"Termienating through signal handler\n",35);
    free(myCommand);
    free(tokens);
    free(tokens2); 
    exit(0);
}

// helper method to exit my program
int my_exit(){
    printf("Goodbye!\n");
    free(myCommand);
    free(tokens);
    free(tokens2);
    exit(0);
}

// helper method that print out all of the built-in commands
int getHelp(){
    printf("===================\n");
    printf("build-in commands: \n");
    int i;
    for (i=0; i<num_built_in; i++){
        printf("%s\n", built_in_commands[i]);
    }
    printf("====================\n");
    return 1;
}

// helper method gets the current working directory
int getpwd(){
    char pwd[BUFFER_SIZE];
    if (getcwd(pwd, sizeof(pwd)) != NULL){
        fprintf(stdout, "%s\n", pwd);
    }else {
        printf("pwd error.\n");
    }
    return 1;
}

// helper method to change current working directory
int changeDirectory(char** args){
    char* directory = (args[1]);
    if (directory == NULL){
        chdir("/home");
        return 1;
    }
    if (chdir(directory) == -1){
        printf("No such directory:%s \n", args[1]);
    }
    return -1;
}

// helper method to play a guessing game
// Users need to answer correctly 5 times before exiting the game
int my_game(){
	int randomNumber;
	int array[5];
	
    int i;
	for (i = 0; i < 5; i++){
	    int guess = 0;
	    int try = 0;
		randomNumber = rand() % 10 + 1;
		printf("Pick a number between 1-10.\n");
	
		while (guess != randomNumber){
			printf("Make a guess:");
			scanf("%d", &guess);
			try++;
			if (guess > randomNumber){
				printf("No guess lower!\n");
			}
			else if (guess < randomNumber){
				printf("No guess higher!\n");
			}
			else {
				printf("you got it!\n");
				array[i] = try;
			}
		}
	}
    int j;
	for (j = 0; j < 5; j++){
		printf("Game %d took you %d guesses\n", j+1 , array[j]);
	}
	
	return 0;
}


// check if a command usered entered is included in build_in command
// return 0 if not included or entered command is NULL
// return 1 if it is included
int checkCommand(char** args){
    if (args == NULL){
        return 0;
    }
    int i;
    for (i=0; i< num_built_in; i++){
        if(strcmp(args[0], built_in_commands[i]) == 0){
            return 1;
        }
    }
    return 0;
}

// Execute build-in commands.
int execute_built_in(char** args){
    int flag;
    if (strcmp(args[0], built_in_commands[0]) == 0) {
        int flag = my_exit();
    }
    else if (strcmp(args[0], built_in_commands[1]) == 0) {
        int flag = getHelp();
    }
    else if (strcmp(args[0], built_in_commands[2]) == 0) {
        int flag = getpwd();
    }
    else if (strcmp(args[0], built_in_commands[3]) == 0) {
        int flag = changeDirectory(args);
    }
    else if (strcmp(args[0], built_in_commands[4]) == 0) {
        int flag = my_game();
    }else{
        printf("Command not found--Did you mean something else?\n");
    }
    return flag;
}

// Execute one command including both buit-in and non built-in commands.
int execute_one_command(char** args){
	if (args == NULL){
		return -1;
	}
	if ((checkCommand(args))) {
		//printf("A build-in command is executed: %s\n", args[0]);
        execute_built_in(args);
    }else{
    	pid_t cpid = fork();
        if (cpid < 0) {
			printf("\nAn error occurred with creating fork\n");
            return -1;
        }
        if (cpid == 0) {
            execvp(args[0], args);
        }
        wait(NULL);
        //printf("DONE WITH PARENT\n");
    }
    return 0;
}

// read user input
// return an array of characters
char* readCommand(void){
    myCommand = malloc(sizeof(char)*BUFFER_SIZE);
    if (!myCommand){ // Buffer Allocation Failed
        printf("\nBuffer Allocation Error. Unable to read.\n");
        free(myCommand);
        exit(1);
    } 
    int i = 0;
    int c;  
    while (c != '\n'){
        c = getchar();
        myCommand[i] = c;
        i++;
    }
    //printf("readCommand:%s\n", command);
    return myCommand;
}

// check if a command contains a pipe.
int checkPipe(char* args){
	//printf("checking pipe in %s\n", args);
	char* p;
	p = strstr(args, "|");
	if (p != NULL){
		//printf("find pipe\n");
		return 1;
	}
	//printf("didn't find pipe\n");
	return 0;
}

// parse a command with pipe
// returns a 2d characters array e.g ""ls","wc""
char** parsePipeCommand(char* args){
	if (args == NULL){
		return NULL;
	}
    int i = 0;
    tokens = (char **)malloc(sizeof(char *) * BUFFER_SIZE);
    if (!tokens){
        free(tokens);
        printf("\nBuffer Allocation Error. Unable to process.\n");
        exit(1);
    }
    char const delim[] = "|";
    char* token = strtok(args, delim);
    while (token != NULL){
        tokens[i] = token;
        //printf("prompt commands include: %s\n", token);
        i ++;
        token = strtok(NULL,delim);
    }
    tokens[i] = NULL;
    return tokens;
}

// parse each command line for execution
char** parseCommand(char* args){
	if (args == NULL){
		return NULL;
	}
    int i = 0;
    tokens2 = (char **)malloc(sizeof(char *) * BUFFER_SIZE);
    if (!tokens2){
        free(tokens2);
        printf("\nBuffer Allocation Error. Unable to process.\n");
        exit(1);
    }
    char* token = strtok(args, " \t\n()<>&;");
    while (token != NULL){
        tokens2[i] = token;
        //printf("prompt commands include: %s\n", token);
        i ++;
        token = strtok(NULL," \t\n()<>&;");
    }
    tokens2[i] = NULL;
    return tokens2;
}

// execute commands with pipe
int execute_pipe(char** first, char** last){
    int fd[2];
    if (pipe(fd) == -1){
        printf("\nPipe Error\n");
        return -1;
    }
    
    pid_t pid1 = fork();
    if (pid1 < 0){
        printf("\nFork Error\n");
        return -1;
    }
    if (pid1 == 0){
        // child process 1 (first command)
        //printf("CHILD1 PROCESS\n");
        dup2(fd[1], STDOUT_FILENO);
        execvp(first[0], first);
    }
    
    pid_t pid2 = fork();
    if (pid2 < 0){
        printf("\nFork Error\n");
        return -1;
    }
    if (pid2 == 0){
        // child process 2 (second command)
        //printf("CHILD2 PROCESS\n");
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(last[0], last);
    }else{
        //printf("PARENT PROCESS\n");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL,0);
        waitpid(pid2, NULL,0);
    }
    return 0;
}

// Check if a input command is valid
// if a command only conatins whitespaces are is not valid
int isValid(char* args){
    if(args == NULL || args[0] == '\n' || args[0] == '\t'
        || args[0] == '\r' || args[0] == '\a'|| args[0] == ' ') {
        return 0;
    }
    return 1;
}

int main(){
    // kill program in 200 seconds
    alarm(200);
	signal(SIGINT, sigint_handler);
	printf("\nWelcome to mini-shell!\n");
    printf("You can terminate anytime by pressing Ctrl+C\n");
    printf("You can also terminate at prompt line by returning exit\n");

    while(1){
    	printf("mini-shell>");
    	char* readline = readCommand();
    	if (readline != NULL && isValid(readline)){
    		if (checkPipe(readline) > 0){
    			char** pipedCommand = parsePipeCommand(readline);
    			char** first = parseCommand(pipedCommand[0]);
    			char** last = parseCommand(pipedCommand[1]);
    			execute_pipe(first, last);
    		}else{
    			char** command = parseCommand(readline);
    			execute_one_command(command);
    		}
		}
    }
    return 0;
}

