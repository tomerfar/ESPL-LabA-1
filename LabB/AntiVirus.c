#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
typedef struct virus {
        unsigned short SigSize;
        char virusName[16];
        unsigned char* sig;

    } virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};

link* virus_list = NULL;
char sigFile[256] = "signatures-L";
char suspectetFileName[256] = "";
char buffer[1024];
bool isBigEndian = false;



//Functions declarations
void SetSigFileName();
virus* readVirus(FILE* file);
void printVirus(virus* v);
void list_print(link* virus_list, FILE* stream);
link* list_append(link* virus_list, virus* data);
void list_free(link* virus_list);

//Functions declarations


//Linked List methods
void list_print(link *virus_list, FILE* list_data){
    link *current = virus_list;
    while(current != NULL){
        printVirus(current->vir);
        current = current->nextVirus;
    }

}

link* list_append(link *virus_list, virus* data){
    link *new_head = malloc(sizeof(link));
    if(new_head == NULL){
        fprintf(stderr, "Failed to allocate memory for new link.\n");
        return virus_list;
    }
    new_head->vir = data;
    new_head->nextVirus = virus_list;

    return new_head;
}

void list_free(link *virus_list){
    link *current = virus_list;
    while(current != NULL){
        link *next = current->nextVirus;
        free(current->vir->sig);
        free(current->vir);
        free(current);
        current = next;
    }
}
//Linked List methods

void SetSigFileName(){
    printf("Enter a new signature file name: ");
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(sigFile, buffer);
    } else {
        printf("Error reading input. Please try again.\n");
    }
}

virus* readVirus(FILE* file){

    virus* vir = (virus*)calloc(1, sizeof(virus)); // remember to free this memory afterwards
    if(vir == NULL){
        fprintf(stderr,"Error: could not allocate memory for virus structure.\n");
        return NULL;
    }
    if(fread(vir, 1, 18 , file) != 18){ /* when read data from file into a struct,
     the data is stored in the fields of the struct in the order in which is read from the file.*/
        free(vir);
        return NULL;
    }
   
    // if(isBigEndian){
    //     vir->SigSize = (vir->SigSize >> 8) | (vir->SigSize << 8);
    // }

    vir->sig = (unsigned char*)calloc(vir->SigSize, sizeof(unsigned char));
    if(vir->sig == NULL){
        fprintf(stderr, "Error: could not allocate memory for virus signature.\n");
        free(vir);
        return NULL;
    }
    if(fread(vir->sig,1,vir->SigSize,file) != vir->SigSize){ 
        fprintf(stderr, "Error in reading virus signature bytes\n");
        free(vir->sig);
        free(vir);
        return NULL; 
    }
    printf("Success\n");
    return vir;
}

void printHex(unsigned char* buffer, int length){
    for(int i = 0; i < length; i++){
        printf("%02X ",buffer[i]);
    }
    printf("\n");
}

void printVirus(virus* virus){ //notice we changed here the signature to be a better fit for us at the moment.
    printf("Virus details:\n");
    printf("virus name: %s\n", virus->virusName);
    printf("virus signature length: %d\n", virus->SigSize);
    printf("virus signature: ");
    printHex(virus->sig, virus->SigSize);
    printf("\n");
}

void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    link *current = virus_list;
    while (current != NULL) {
        for (unsigned int i = 0; i < size - current->vir->SigSize; i++) {
            if (memcmp(buffer + i, current->vir->sig, current->vir->SigSize) == 0) {
                printf("Virus found!\n");
                printf("Starting byte that virus detected: %u\n", i);
                printf("Virus name: %s\n", current->vir->virusName);
                printf("Virus signature size: %u\n", current->vir->SigSize);
            }
        }
        current = current->nextVirus;
    }
}

void FixFile() {
    printf("FixFile function not implemented\n");
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
    bool isLoaded = false;; // flag to check whether we already loaded the signatures.
    while(1){ // infinite loop
        printf("Menu:\n");
        printf("0) Set signatures file name\n");
        printf("1) Load signatures\n");
        printf("2) Print signatures\n");
        printf("3) Detect viruses\n");
        printf("4) Fix file\n");
        printf("5) Quit\n");
        printf("Please select a function:\n");

        char input[10];
        int option;
        if(fgets(input, sizeof(input), stdin) == NULL){ //EOF
            break;
        }
        if(sscanf(input, "%d", &option) != 1){
            option = -1; //symbol for invalid option
        }

        switch(option){
            case 0: {
                SetSigFileName();
                break;
            }
            case 1:{
                FILE* file = fopen(sigFile, "rb");
                if(file == NULL){
                    fprintf(stderr,"Error: could not open file %s\n", sigFile);
                    return 1;
                }
                char magic_buffer[4]; // 4 bytes for magic number and 1 for null terminator
                if(fread(magic_buffer, 1, 4 , file) != 4){
                    fprintf(stderr, "Error: could not read magic number from file %s", sigFile);
                    fclose(file);
                    break;
                }

                if (memcmp(magic_buffer, "VIRL", 4) != 0 && memcmp(magic_buffer, "VIRB", 4) != 0) {
                    fprintf(stderr, "Error: incorrect magic number in file %s", sigFile);
                    fclose(file);
                    break;
                }
                // else if(memcmp(magic_buffer, "VIRL", 4) == 0){
                //     isBigEndian = false;
                // }
                // else if(memcmp(magic_buffer, "VIRB", 4) == 0){
                //     isBigEndian = true;
                // }
                virus* vir;
                while((vir = readVirus(file)) != NULL){
                    virus_list = list_append(virus_list, vir);
                }
                isLoaded = true;
                fclose(file);
                break; 
            }
            case 2:{
                if(isLoaded){
                    list_print(virus_list, stdout);
                }
                else{
                    fprintf(stderr,"Error: viruses aren't loaded.\n");
                }
                break;
            }
            case 3:{
                FILE* file = fopen(suspectetFileName, "rb");
                if(file == NULL){
                        fprintf(stderr,"Error: could not open file %s\n", suspectetFileName);
                        break;;
                }
                char buffer[10000];
                unsigned int size = fread(buffer,1, sizeof(buffer), file);
                fclose(file);
                if(size == 0){
                    fprintf(stderr, "Error: failed to read file.\n");
                    break;
                }

                detect_virus(buffer, size, virus_list);
                break;
            }
            case 4:{
                FixFile();
            }
            case 5:{
                list_free(virus_list);
                exit(0);
            }
            default:
            printf("Invalid option\n");
            break;


        }
   
    }

    return 0;



}