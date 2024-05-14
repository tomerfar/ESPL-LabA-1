#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char apply_encoding(char user_input, char encoding_arg[], char operation, int key_index) {
    char ch  = user_input;

    if (operation == '+') {
        if (user_input >= 'a' && user_input <= 'z') {
            ch = (user_input - 'a' + encoding_arg[key_index] - '0') % 26 + 'a';
        } else if (user_input >= '0' && user_input <= '9') {
            ch = (user_input - '0' + encoding_arg[key_index] - '0') % 10 + '0';
        }
    } else if (operation == '-') {
        if (user_input >= 'a' && user_input <= 'z') {
            ch = (user_input - 'a' - encoding_arg[key_index] + '0' + 26) % 26 + 'a';
        } else if (user_input >= '0' && user_input <= '9') {
            ch = (user_input - '0' - encoding_arg[key_index] + '0' + 10) % 10 + '0';
        }
    }
    return ch;
}

int main(int argc, char* argv[]) {
//Debug Mode
     int debug_mode = 1; //default mode of debug is set to "on"
    for(int i = 0; i < argc; i++){
        //updating debug mode if we passed "-D", "+D" as an argument
        if(strcmp(argv[i], "-D") == 0){ //the argument that we passed is -D
            debug_mode = 0;
        }
        else if(strcmp(argv[i], "+D") == 0){
            debug_mode = 1;
        }
        //prints the arguments if in debug mode
        if(debug_mode){ //true if non-zero value
            fprintf(stderr, "Arguemnt %d: %s\n", i, argv[i]);
            
        }
    }
   
  //Encoder
  char* encoding_arg = NULL;
  char operation;
  int encodeArgument = 0; //signals whether one of the arguments in the command line was an encoder
  for(int i = 1; i < argc; i++){ //search for encoder as an argument by his operator, if exist
    if((argv[i][0] == '+' || argv[i][0] == '-') && argv[i][1] == 'e'){
        encoding_arg = argv[i];
        operation = argv[i][0];
        encodeArgument = 1;
        break;
    }
  }
    int length = 0;
    if(encodeArgument == 1){
        while (encoding_arg[length] != '\0') { // calculating the length of the encoder argument
            length++;
        }
    }
    int key_index = 2; // Skips +e/-e

    //Check if needs to read/write to/from another file
    FILE* infile = stdin; //default
    FILE* outfile = stdout; //default
    
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-I") == 0 && i + 1 <= argc){ //checks if there is a filename as an argument as the next argument
            infile = fopen(argv[i+1], "r");
        
            if(infile == NULL){
                fprintf(stderr, "Error: Unable to open input file \n");
                return 1;
            }
        } else if (strcmp(argv[i], "-O") == 0 && i + 1 <= argc){ //checks if there is a filename as an argument as the next argument
            outfile = fopen(argv[i + 1], "w");
            if(outfile == NULL){
                fprintf(stderr, "Error: Unable to open output file \n");
                return 1;
            }
        }

    } 
    
    int c;
    if(encodeArgument == 1){
        while ((c = fgetc(infile)) != EOF) {
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                char ch = apply_encoding(c, encoding_arg, operation, key_index);
                fputc(ch, outfile);
            } else {
                // not a lowercase letter nor a digit number
                fputc(c, outfile);
                }   
            key_index = (key_index + 1) % length;
            if (key_index == 0 || key_index == 1) {
                key_index = 2; // Skips +e/-e
            }
        }
    }

    fclose(infile);
    fclose(outfile);

    return 0;
}
