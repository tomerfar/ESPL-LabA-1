#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>

void handler(int sig)
{
    printf("\nReceived Signal : %s\n", strsignal(sig));
    signal(sig, SIG_DFL);
    raise(sig);

    if (sig == SIGCONT) {
        signal(SIGTSTP, handler);
    } else if (sig == SIGTSTP) {
        signal(SIGCONT, handler);
    }
}

int main(int argc, char **argv)
{
    printf("Starting the program\n");
    signal(SIGINT, handler);
    signal(SIGTSTP, handler);
    signal(SIGCONT, handler);

    while (1)
    {
        sleep(1);
    }

    return 0;
}