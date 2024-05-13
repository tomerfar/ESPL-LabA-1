#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    char input[256]; // an array which saves the user input
    int i = 0;
    int c = 0;
    printf("Insert something and i will echo it: \n");
    fflush(stdout);

    while((c = fgetc(stdin)) != '\n'){
        input[i] = (char)c;
        i++; 
    }
    i = 0;
    printf("%s %s\n", input, input);


    return 0;

}