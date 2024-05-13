#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv){
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

    return 0;
}