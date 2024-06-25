
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>



int main(int argc, char **argv){

char *command1[] = {"ls", "-l", NULL};
char *command2[] = {"tail", "-n", "2", NULL};
int pipefd[2];
pid_t cpid1, cpid2;

 if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "(parent_process>forking...)\n");
    cpid1 = fork(); // forking the first child process
    if (cpid1 == -1) {
        perror("Error: couldn't fork");
        exit(EXIT_FAILURE);
    }

    if(cpid1 == 0){ //enters the child process
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        fprintf(stderr, "(child1>going to execute cmd: ls -l…)\n");
        execvp(command1[0], command1);
        perror("execvp on first child");
        exit(EXIT_FAILURE); 
    }
    else
    {
       fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid1);
        close(pipefd[1]); // Close write end of pipe
        fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
 
    }

    cpid2 = fork(); //forking the second child
    if (cpid2 == -1) {
        perror("Error: couldn't fork");
        exit(EXIT_FAILURE);
    }

    if (cpid2 == 0){
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]); // Close the read end after duplicating
        fprintf(stderr, "(child2>going to execute cmd: tail -n 2…)\n");
        execvp(command2[0], command2);
        perror("execvp on second child");
        exit(EXIT_FAILURE);
        
    }
    else
    {
        fprintf(stderr, "(parent_process>created process with id: %d)\n", cpid2);
        fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
        close(pipefd[0]); // Close read end of pipe
        fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
        waitpid(cpid1, NULL, 0); // Wait for child 1
        waitpid(cpid2, NULL, 0); // Wait for child 2
        fprintf(stderr, "(parent_process>exiting…)\n");

    }
    return 0;
    
    
}

