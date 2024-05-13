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
        else{
            //prints the arguments if in debug mode
              if(debug_mode){ //true if non-zero value
            fprintf(stderr, "Arguemnt %d: %s\n", i, argv[i]);
            }
        }
              
   }
   
  //Encoder
    char* encoding_arg = argv[1];
    char operation = encoding_arg[0];
    int length = 0;
    while (encoding_arg[length] != '\0') { // calculating the length of the encoder argument
        length++;
    }
    FILE* infile = stdin; //default
    FILE* outfile = stdout; //default
    
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-I" == 0) && i + 1 < argc){ //checks if there is a filename as an argument as the next argument
            infile = fopen(argv[i+1], "r");
        
        if(infile == NULL){
            fprintf(stderr, "Error: input file is Null \n");
            return 1;
        }
        } else if (strcmp(argv[i], "-O") == 0 && i + 1 < argc){ //checks if there is a filename as an argument as the next argument
            outfile = fopen(argv[i + 1], "w");
            if(outfile == NULL){
                fprintf(stderr, "Error: output file is Null \n");
                return 1;
            }
        }

        } 
    

    int key_index = 2; // Skips +e/-e
    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            char ch = apply_encoding(c, encoding_arg, operation, key_index);
            fputc(ch, stdout);
        } else {
            // not a lowercase letter nor a digit number
            fputc(c, stdout);
        }
        key_index = (key_index + 1) % length;
        if (key_index == 0 || key_index == 1) {
            key_index = 2; // Skips +e/-e
        }
    }

    fclose(infile);
    fclose(outfile);

    return 0;
}