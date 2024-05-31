#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> // For getcwd
#include <sys/wait.h>
#include "LineParser.c"
#include <limits.h> // For PATH_MAX

#define PATH_MAX 4096
#define MY_MAX_INPUT 2048

void execute(cmdLine *pCmdLines)
{
   pid_t child_pid; /* child process id*/
   if((child_pid = fork()) == -1){ 
    perror("fork() error");
    exit(1);
   }
   else if(child_pid == 0){
    if(execvp(pCmdLines->arguments[0], pCmdLines->arguments) == -1){ /* execv needs to recieve the full path name, while execvp needs only to recieve the filename, and then it searches this name inside the cd*/
      perror("execvp() error");
      exit(1);  
    }
   }
    else{ //creates a child process that runs conccurently with the parent process.
        if(pCmdLines->blocking){
            waitpid(child_pid, NULL, 0);
        }


    }
}


int main(int argc, char **argv)
{
    char cwd[PATH_MAX];
    char input[MY_MAX_INPUT];
    cmdLine* parseCmd;
    while(1){ /* infinite loop*/

        if(getcwd(cwd, sizeof(cwd)) != NULL){
            printf("%s\n ", cwd);
        }
        else{
            perror("getcwd() error");
            return 1;
        }
        if(fgets(input, sizeof(input), stdin) == NULL){
            perror("Error in reading the input");
            return 1;
        }
        if (strcmp(input, "quit\n") == 0) {
        printf("Exiting myshell.\n");
        break;
        }

        parseCmd = parseCmdLines(input);
        if(parseCmd == NULL){
            perror("Error in parsing command");
            continue;
        }
        execute(parseCmd);
        freeCmdLines(parseCmd);
    }

    return 0;
}

