#include <stdio.h>
#include <stdlib.h>

void printHex(unsigned char* buffer, int length){
    for(int i = 0; i < length; i++){
        printf("%02X ",buffer[i]);
    }
    printf("\n");
}


int main(int argc, char **argv)
{
    if(argc == 1){
        fprintf(stderr, "Error: filename has not been provided. closing program.\n");
        return 1;
    }
    else if(argc > 2){
        fprintf(stderr, "Error: too many arguments in command line. closing program.\n");
        return 1;
    }
    FILE* file = fopen(argv[1], "rb");
    if(file == NULL){
        fprintf(stderr, "Error: could not open file %s.\n", argv[1]);
        return 1;
    }
    unsigned char buffer[1024];
    size_t bytes_read = fread(buffer,1,sizeof(buffer),file); /* function reads the data to buffer, each byte is size 1, reads as much as buffer can hold, and reads from file*/
    if(bytes_read < sizeof(buffer)){
        if(feof(file)){
            printf("End of file reached.\n");
        }
        else if(ferror(file)){
            fprintf(stderr, "Error reading from file.\n");
        }
    }
    printHex(buffer, bytes_read);
    fclose(file);


    return 0;
}


