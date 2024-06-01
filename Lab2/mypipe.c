#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> // For getcwd
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <limits.h> // For PATH_MAX
#include <fcntl.h>
#include <linux/stat.h>

int main(int argc, char **argv)
{
    char buf;
    int p[2];
    pipe(p); //creates a one way communication channel. in p[1] we write, and it sends to p[1].
    pid_t pid = fork();
    if(pid > 0){
        printf("parent id: %d\n",pid);
        close(p[0]);
        write(p[1], "hello", sizeof("hello"));
        close(p[1]);
        wait(NULL);
    }
    else{
        printf("child id: %d \n", pid);
        close(p[1]);
        while (read(p[0],&buf,1) != 0)
        {
            putchar(buf);
        }
        close(p[0]);
        printf("\nfinished reading\n");
    }
    return 0;



}