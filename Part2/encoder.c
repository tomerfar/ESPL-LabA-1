#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char apply_encoding(char user_input, char encoding_arg[], char operation, int key_index) {
    char ch;

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
    if (argc != 2) {
        printf("invalid input, can only insert 2 arguments each time \n");
        return 1;
    }

    char* encoding_arg = argv[1];
    char operation = encoding_arg[0];
    // char encoder[] = NULL;
    // int length = 0;
    // for(int i = 2; i < encoding_arg; i++){
    //     encoder[i-2] = encoding_arg[i];
    //     length ++; 
    // }
    //char* encoding_key = encoding_arg + 2; // Skip the "+e" or "-e"
    int key_length = strlen(encoding_arg);
    int key_index = 2;

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            char ch = apply_encoding(c, encoding_arg, operation, key_index);
            fputc(ch, stdout);
            key_index = (key_index + 1) % key_length;
            if((key_index == 0) | (key_index == 1)){
                key_index = 2;
            }
        } else {
            // not a lowercase letter nor a digit number
            fputc(c, stdout);
        }
    }
    
    return 0;
}
