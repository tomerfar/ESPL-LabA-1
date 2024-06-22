
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>



int main(int argc, char **argv){
char files_list[1000];

int pipefd[2];
pid_t cpid1, cpid2;

 if (pipe(pipefd) == -1) {
        perror("pipe error");
        exit(EXIT_FAILURE);
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
        //execvp("ls", "-l" ,//string array);
    }

    close(pipefd[1]);
    cpid2 = fork();
    if (cpid1 == -1) {
        perror("Error: couldn't fork");
        exit(EXIT_FAILURE);
    }

    if (cpid2 == 0){
        close(STDIN_FILENO);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        //excevp

    }
    close(pipefd[0]);
    waitpid(cpid1, NULL, 0);
    waitpid(cpid2, NULL, 0);
    return 0;
    
    
}

