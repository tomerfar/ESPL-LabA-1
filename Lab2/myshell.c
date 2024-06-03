#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> // For getcwd
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "LineParser.h"
#include <signal.h>
#include <fcntl.h>
#include <linux/stat.h>
#include <linux/limits.h> // For PATH_MAX

//#define PATH_MAX 4096
#define MY_MAX_INPUT 2048

void handleAlarm(int process_id){
    
    if(kill(process_id, SIGCONT) == -1){
        fprintf(stderr, "failed to send SIGCONT sign\n");
    }
    else{
        printf("SIGCONT sign was sent to process: %d\n", process_id);
    }
}

 void handleBlast(int process_id){
    if(kill(process_id, SIGKILL) == -1){
        fprintf(stderr, "failed to send SIGKLILL sign\n");
    }
    else{
        printf("SIGKILL sign was sent to process: %d\n", process_id);
    }

}

void execute(cmdLine *pCmdLines, int debug)
{
   pid_t child_pid; /* child process id*/
   if((child_pid = fork()) == -1){ //couldn't fork
    perror("fork() error");
    exit(1);
   }
   else if(child_pid == 0){
    if(pCmdLines->inputRedirect != NULL){
        int fd = open(pCmdLines->inputRedirect, O_RDONLY);
        if(fd == -1){
            perror("open() error");
            _exit(1);
        }
            if(dup2(fd,STDIN_FILENO) == -1){
            perror("dup2() error");
            _exit(1);
        }
            close(fd);
    }
    if(pCmdLines->outputRedirect != NULL){
        
        int fd = open(pCmdLines->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC ,S_IRUSR | S_IWUSR);
        if(fd == -1){
            perror("open() error");
            _exit(1);
        }
        if(dup2(fd,STDOUT_FILENO) == -1){
            perror("dup2() error");
            _exit(1);
        }
        close(fd);
    }
    if(execvp(pCmdLines->arguments[0], pCmdLines->arguments) == -1){ /* execv needs to recieve the full path name, while execvp needs only to recieve the filename, and then it searches this name inside the cd*/
      perror("execvp() error");
      _exit(1);  
    }
   }
   else{ //creates a child process that runs conccurently with the parent process.
        if(debug){
            fprintf(stderr, "PID: %d\n", child_pid);
            fprintf(stderr,"Executing Command:%s\n", pCmdLines->arguments[0]);
        }

        if(pCmdLines->blocking){ //blocking is 1 
            waitpid(child_pid, NULL, 0);
        }
    }
}


int main(int argc, char **argv)
{
    char cwd[PATH_MAX];
    char input[MY_MAX_INPUT];
    cmdLine* parseCmd;

    int debug = 0;
    for(int i = 0; i < argc; i++){
        //updating debug mode if we passed "-d"
        if(strcmp(argv[i], "-d") == 0){ //the argument that we passed is -d
            debug = 1;
        }
    }
     
    while(1){ /* infinite loop*/
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            printf("%s\n ", cwd); //check in the frontal lab may need to go inside the while loop
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
        printf("Exiting shell.\n");
        break;
        }

        parseCmd = parseCmdLines(input);
        if(parseCmd == NULL){
            perror("Error in parsing command");
            continue;
        }
        /* checks the type of command the shell recieved and reacts accordingly */
        if(strcmp(parseCmd->arguments[0], "cd") == 0){ //change directory
            //printf("argCount: %d\n", parseCmd->argCount);
            if(parseCmd->argCount < 2){
                fprintf(stderr, "Error: file or directory name is missing\n");
            }
            else if(chdir(parseCmd->arguments[1]) == -1){ /* checks if the next argument is a name of an actual file */
                fprintf(stderr, "cd: %s: No such file or directory \n", parseCmd->arguments[1]);
            }
            freeCmdLines(parseCmd);
            continue; /* cd command is handled internally by the shell and doesn't require creating a new process or executing any other command*/
        }
        else if(strcmp(parseCmd->arguments[0], "alarm") == 0){ //wakes up a sleeping process
            if(parseCmd->argCount < 2){
                fprintf(stderr, "alarm: pid is missing\n");
            }
            else{
                int pid = atoi(parseCmd->arguments[1]);
                handleAlarm(pid);
            }
            freeCmdLines(parseCmd);
            continue;
        }
        else if(strcmp(parseCmd->arguments[0], "blast") == 0){ //terminates a running/sleeping process
            if(parseCmd->argCount < 2){
                fprintf(stderr, "blast: pid is missing\n");
            }
            else{
                int pid = atoi(parseCmd->arguments[1]);
                handleBlast(pid);
            }
            freeCmdLines(parseCmd);
            continue;
        }


        execute(parseCmd, debug); //if the commands is neither of cd alarm or blast, it handles different commands that the user can ask for
        freeCmdLines(parseCmd);
    }

    return 0;
}

