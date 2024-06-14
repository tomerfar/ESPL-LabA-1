#include "Util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define SYS_EXIT 1

extern int system_call();

extern void infection();
extern void infector(const char *filename);

int main (int argc , char* argv[], char* envp[]){
    if (argc != 2) {
        // Print error message and exit
        system_call(SYS_EXIT, 0x55);
        __asm__("mov $0x55, %eax; int $0x80"); // exit with code 0x55
    }

    const char *arg = argv[1];
    if (arg[0] != '-' || arg[1] != 'a') {
        // Print error message and exit
        system_call(SYS_EXIT, 0x55);
        __asm__("mov $0x55, %eax; int $0x80"); // exit with code 0x55
    }

    const char *filename = argv[1] + 2; // Skip the "-a"
    system_call(SYS_WRITE, STDOUT, filename, strlen(filename));
    system_call(SYS_WRITE, STDOUT, "\n", 1);


    // Call stub functions
    infection();
    infector(filename);

    return 0;
}
