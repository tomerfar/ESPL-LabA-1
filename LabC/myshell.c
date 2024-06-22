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

void executePipe(cmdLine *pCmdLines, int debug){
    int pipefd[2];
    pid_t cpid1, cpid2;
    int saved_stdin = dup(STDIN_FILENO);
    int saved_stdout = dup(STDOUT_FILENO);

    if(pCmdLines -> outputRedirect != NULL){
        fprintf(stderr, "Error: Output redirection '>' not allowed for this command.\n");
        _exit(1);
    }
    if(pCmdLines->next -> inputRedirect != NULL){
        fprintf(stderr, "Error: Input redirection '<' not allowed for this command.\n");
        _exit(1);
    }
     if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }

    if(pCmdLines -> inputRedirect != NULL){
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
    cpid1 = fork();
    if (cpid1 == -1) {
        perror("Error: couldn't fork");
        exit(EXIT_FAILURE);
    }

    if(cpid1 == 0){ //enters the child process
        close(STDOUT_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        if(execvp(pCmdLines->arguments[0], pCmdLines->arguments) == -1){ /* execv needs to recieve the full path name, while execvp needs only to recieve the filename, and then it searches this name inside the cd*/
        perror("execvp() error inside cpid1");
        _exit(1);  
        }
        if(debug){
            fprintf(stderr, "PID: %d\n", cpid1);
            fprintf(stderr,"Executing Command:%s\n", pCmdLines->arguments[0]);
        }
    }

    close(pipefd[1]);

    if(pCmdLines->next -> outputRedirect != NULL){
         int fd = open(pCmdLines->next ->outputRedirect, O_WRONLY);
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

    cpid2 = fork();
    if (cpid2 == -1) {
        perror("Error: couldn't fork");
        exit(EXIT_FAILURE);
    }
    
     if (cpid2 == 0){
        close(STDIN_FILENO);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        if(execvp(pCmdLines->next->arguments[0], pCmdLines->next->arguments) == -1){ /* execv needs to recieve the full path name, while execvp needs only to recieve the filename, and then it searches this name inside the cd*/
            perror("execvp() error inside cpid2");
            _exit(1);  
        }
         if(debug){
            fprintf(stderr, "PID: %d\n", cpid2);
            fprintf(stderr,"Executing Command:%s\n", pCmdLines->next->arguments[0]);
        }
    }

    close(pipefd[0]);
    //if(pCmdLines->blocking){
        waitpid(cpid1, NULL, 0);
        waitpid(cpid2, NULL, 0);
    //}
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdin);
        close(saved_stdout);
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
            perror("Errors in reading the input");
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
        else if(parseCmd->next != NULL){
            executePipe(parseCmd, debug);
        }

        else if(parseCmd->next == NULL)
        {
            execute(parseCmd, debug);
        }
        //if the commands is neither of cd alarm or blast, it handles different commands that the user can ask for
        freeCmdLines(parseCmd);
    }

    return 0;
}

