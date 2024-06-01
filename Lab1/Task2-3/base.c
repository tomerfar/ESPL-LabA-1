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
  mapped_array[array_length] = '\0';
 
  return mapped_array;
}

char addOp(char c){
    return c + 1; //increment ascii value of the character by 1.
}

//Task 2b
char my_get(char c){
  /* Ignores c, reads and returns a character from stdin using fgetc. */
  return fgetc(stdin);
}

char cprt(char c){
  if((c >= 0x20) && (c <= 0x70)){
    printf("ASCII Value of char:%c\n", c);
  }
  else{
    putchar('.');
    putchar('\n');
  }
  return c;
}

char encrypt(char c){
  if((c >= 0x20) && (c <= 0x4E)){
    c = c + 0x20;
  }
  return c;
}

char decrypt(char c){
  if((c >= 0x40) && (c <= 0x7E)){
    c = c - 0x20;
  }
  return c;
}

char xoprt(char c) {
    printf("Hex: 0x%X\n", c);
    printf("Oct: 0%o\n", c);
    putchar('\n');
    return c;
}


 
int main(int argc, char **argv){
  /* TODO: Test your code */
  int base_len = 5;
  char arr1[base_len];
  char* arr2 = map(arr1, base_len, my_get);
  char* arr3 = map(arr2, base_len, cprt);
  char* arr4 = map(arr3, base_len, xoprt);
  char* arr5 = map(arr4, base_len, encrypt);
  free(arr2);
  free(arr3);
  free(arr4);
  free(arr5);

}


