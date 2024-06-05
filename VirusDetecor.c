#include <stdio.h>
#include <stdlib.h>
#include "hexaPrint.c"

typedef struct virus {
        unsigned short SigSize;
        char virusName[16];
        unsigned char* sig;

    } virus;

char* sigFile = "signatures.L";
char buffer[1024];
int isBigEndian = 0; //flag to check which endian it is


void SetSigFileName(){
    printf("Enter a new signature file name: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    sigFile = buffer;


}

virus* readVirus(FILE* file){

    virus* vir = malloc(sizeof(virus)); // remember to free this memory afterwards
    if(vir == NULL){
        fprintf(stderr,"Error: could not allocate memory for virus structure.\n");
        return NULL;
    }
    unsigned short length; //unsigned is for holding only non negative integers.
    if(fread(&length, 2, 1 , file) != 1){
        fprintf(stderr, "Error in reading length bytes\n");
        free(vir);
        return NULL;
    }
    // If the machine is big-endian, swap the bytes of the length
    if (is_big_endian()) {
        length = (length >> 8) | (length << 8);
    }

    vir->SigSize = length;
    if(fread(vir->virusName,16,1,file) != 1){
         fprintf(stderr, "Error in reading virus name bytes\n");
         free(vir);
         return NULL;
    }
    vir->sig = malloc(length);
    if(vir->sig == NULL){
        fprintf(stderr, "Error: could not allocate memory for virus signature.\n");
        return NULL;
    }
    if(fread(vir->sig,length,1,file) != 1){ 
        fprintf(stderr, "Error in reading virus signature bytes\n");
        free(vir->sig);
        free(vir);
        return NULL; 
    }

    return vir;
}

void printVirus(virus* virus){
    printf("Virus details:\n");
    printf("virus name: %s\n", virus->virusName);
    printf("virus signature length: %d\n", virus->SigSize);
    printf("virus signature: ");
    printHex(virus->sig, virus->SigSize);
    printf("\n");
}

int is_big_endian() { //helper function to check whether we read in big/little endian
    int test = 1;
    char *p = (char*)&test;
    return p[0] == 0;
}

int main(int argc, char **argv)
{
    if(argc < 2){
        fprintf(stderr, "Error: no file name provided.\n");
        exit(1);
    }
    FILE *file = fopen(argv[1], "rb"); // opening file in reading bytes mode.
    if(file == NULL){
        fprintf(stderr, "Error: could not open file %s", argv[1]);
        exit(1);
    }

    




    fclose(file);
    return 0;
}