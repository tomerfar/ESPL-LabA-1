#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* map(char *array, int array_length, char (*f) (char)){
     /* TODO: Complete during task 2.a */
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  if(mapped_array == NULL){
    return NULL;
  }
  for(int i = 0 ; i < array_length; i++){
    mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}

char my_get(char c){
  /* Ignores c, reads and returns a character from stdin using fgetc. */
  return fgetc(stdin);
}

char cprt(char c){
  if((c >= (char)0x20) && (c <= (char)0x7E)){
    printf("%c\n", c);
  }
  else{
    putchar('.');
    putchar('\n');
  }
  return c;
}

char encrypt(char c){
  if(c >= 0x20 && c <= 0x4E){
    c = c + 0x20;}
  return c;
}

char decrypt(char c){
  if((c >= 0x40) && (c <= 0x7E)){
    c = c - 0x20;
  }
  return c;
}

char xoprt(char c) {
     printf("Hex:%x Oct:%o\n", c, c);
    return c;
}

struct fun_desc {
char *name;
char (*fun)(char);
};


int main(int argc, char **argv){
    struct fun_desc menu[] = {{"Get String", my_get}, {"Print String", cprt} , {"Encryption", encrypt}, {"Decryption", decrypt}, {"Print Hex/Oct", xoprt}, {NULL, NULL}};
    
    int base_len = 5;
    char *carray = (char *)malloc(base_len * sizeof(char));
    for (int i = 0; i < base_len; i++) {
        carray[i] = '\0';
    }
    int numOfItems = sizeof(menu) / sizeof(menu[0]) - 1;
    while(1){ //Loops forever or until it gets an EOF
    printf("Select operation from the following menu:((ctrl^D for exit)\n");

    for(int i = 0; menu[i].name != NULL; i++){
        printf("%d) %s\n", i , menu[i].name);
    }

    printf("Choose a function by its number in the menu:\n");
    char input[10];
    if(fgets(input, sizeof(input), stdin) == NULL){ //EOF
        break;
    }
    else{
        int option = atoi(input);
    
        if (option >= 0 && option < numOfItems) {
            printf("Within bounds\n");
            carray = map(carray, base_len, menu[option].fun);
        } else {
            printf("Not within bounds\n");
            return 1;
        }
    }
    printf("Done.\n");


    }
    free(carray); //free memory allocation
    return 0;
}