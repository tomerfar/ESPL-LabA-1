#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> // For getcwd
#include <limits.h> // For PATH_MAX
#include <sys/wait.h>
#include "LineParser.c"

int main(int argc, char **argv)
{
    char input[2048]; /* maximum size of an input*/
    cmdLine* parseCmd;
    while(1){ /* infinite loop*/
        char cwd[_PC_PATH_MAX];
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            printf("%s> ", cwd);
        }
        else{
            perror("getcwd() error");
            return 1;
        }
        if(fgets(input, sizeof(input), stdin) == NULL){
            perror("Error in reading the input");
            return 1;
        }
        parseCmd = parseCmdLines(input);
        if(parseCmd == NULL){
            perror("Error in parsing command");
            continue;
        }
        else{
            execute(&parseCmd);
            freeCmdLines(parseCmd);
        }
    }

    return 0;
}

void execute(cmdLine *pCmdLines)
{
   int pid;
   if((pid = fork()) == -1){ 
    perror("fork() error");
    exit(1);
   }
   else if(pid == 0){
    if(execv(pCmdLines->arguments[0], pCmdLines->arguments) == -1){
      perror("execv() error");
      exit(1);  
    }
   }
    else{
        if(!pCmdLines->blocking){
            waitpid(pid, NULL, 0);
        }


    }
}
