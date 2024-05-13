#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char apply_encoding(char user_input, char encoding_key, char operation, int key_index) {
    char ch;

    if (operation == '+') {
        if (user_input >= 'a' && user_input <= 'z') {
            ch = (user_input - 'a' + encoding_key[key_index] - '0') % 26 + 'a';
        } else if (user_input >= '0' && user_input <= '9') {
            ch = (user_input - '0' + encoding_key[key_index] - '0') % 10 + '0';
        }
    } else if (operation == '-') {
        if (user_input >= 'a' && user_input <= 'z') {
            ch = (user_input - 'a' - encoding_key[key_index] + '0' + 26) % 26 + 'a';
        } else if (user_input >= '0' && user_input <= '9') {
            ch = (user_input - '0' - encoding_key[key_index] + '0' + 10) % 10 + '0';
        }
    }
    return ch;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [+e1234 or -e13061]\n", argv[0]);
        return 1;
    }

    char* encoding_arg = argv[1];
    char operation = encoding_arg[0];
    char* encoding_key = encoding_arg + 2; // Skip the "+e" or "-e"
    int key_length = strlen(encoding_key);
    int key_index = 0;

    int c;
    while ((c = fgetc(stdin)) != EOF) {
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
            char modified_char = apply_encoding(c, encoding_key, operation, key_index);
            fputc(modified_char, stdout);
            key_index = (key_index + 1) % key_length;
        } else {
            // Non-letter, non-digit character
            fputc(c, stdout);
        }
    }

    // Close the output stream
    fclose(stdout);
    return 0;
}
