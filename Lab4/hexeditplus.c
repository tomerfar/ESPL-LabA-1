#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
  char debug_mode;
  char file_name[100];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_mode;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

struct fun_desc {
char *name;
void (*fun)(state*);
};


static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
/* Structures and static arrays*/

/*Functions*/
void setFileName(state* s){
    printf("Enter file name: ");
    fgets(s->file_name, sizeof(s->file_name), stdin);
    s->file_name[strcspn(s->file_name, "\n")] = '\0';  // Remove newline character
    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void setUnitSize(state* s){
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);

    // Clear input buffer to consume extra characters including newline
    while(getchar() != '\n');

    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Invalid unit size\n");
    }
}

    
void loadMemory(state* s){
    if (strlen(s->file_name) == 0) {
        printf("Error: file name is empty\n");
        return;
    }

    FILE* file = fopen(s->file_name, "rb");
    if (!file) {
        printf("Error: could not open file '%s'\n", s->file_name);
        return;
    }

    printf("Please enter <location> <length>: ");
    char input[100];
    fgets(input, sizeof(input), stdin);
    unsigned int location;
    int length;
    sscanf(input, "%x %d", &location, &length);

    if (s->debug_mode) {
        fprintf(stderr, "Debug: file_name='%s', location=0x%x, length=%d\n", s->file_name, location, length);
    }

    fseek(file, location, SEEK_SET);
    s->mem_count = fread(s->mem_buf, s->unit_size, length, file);

    if (s->mem_count != length) {
        printf("Error: could not read the expected number of units\n");
    } else {
        printf("Loaded %u units into memory\n", s->mem_count);
    }

    fclose(file);
}

void toggleDisplay(state* s){
     if (s->display_mode) {
        printf("Display flag now off, decimal representation\n");
        s->display_mode = 0;
    } else {
        printf("Display flag now on, hexadecimal representation\n");
        s->display_mode = 1;
    }
}

void memoryDisplay(state* s){
     printf("Enter address and length:\n");
    char input[100];
    fgets(input, sizeof(input), stdin);
    unsigned int address;
    int length;
    sscanf(input, "%x %d", &address, &length);

    unsigned char* start;
    if (address == 0) { // special case, start reading from mem_buf[0] no matter what
        start = s->mem_buf;
    } else {
        start = s->mem_buf + address;
    }

    printf("%s\n", s->display_mode ? "Hexadecimal" : "Decimal");
    printf("%s\n", s->display_mode ? "===========" : "=======");

    for (int i = 0; i < length; i++) {
        unsigned int val = 0;
        memcpy(&val, start + i * s->unit_size, s->unit_size);
        if (s->display_mode) {
            printf(hex_formats[s->unit_size - 1], val);
        } else {
            printf(dec_formats[s->unit_size - 1], val);
        }
    }
}

void saveIntoFile(state* s){
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: No file name set.\n");
        return;
    }

    char input[256];
    unsigned int source_address;
    unsigned int target_location;
    int length;

    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %x %d", &source_address, &target_location, &length);

    if (s->debug_mode) {
        fprintf(stderr, "Debug: source_address=%x, target_location=%x, length=%d\n", source_address, target_location, length);
    }

    if (source_address == 0) {
        source_address = (unsigned int)(s->mem_buf);
    }

    FILE *file = fopen(s->file_name, "r+b"); // open to read and write from binary without truncating
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (target_location + (length * s->unit_size) > file_size) {
        printf("Error: Target location is beyond file size.\n");
        fclose(file);
        return;
    }

    fseek(file, target_location, SEEK_SET);

    size_t write_size = length * s->unit_size;
    unsigned char *source = (unsigned char *)source_address;

    size_t written = fwrite(source, s->unit_size, length, file);
    if (written != length) {
        perror("Error writing to file");
    } else {
        printf("Successfully wrote %d units into file.\n", length);
    }

    fclose(file);
}

void debug(state* s){ 
    s->debug_mode = !s->debug_mode;
    printf("Debug flag now %s\n", s->debug_mode ? "on" : "off");
}


void memoryModify(state* s){
    printf("Not implemented yet\n");
}

void quit(state* s){
    if (s->debug_mode) {
        printf("quitting\n");
    }
    exit(0);
}




int main(int argc, char **argv){

     state s = {0, "", 1, {0}, 0, 0};

     struct fun_desc menu[] = {
        {"Toggle Debug mode", debug},
        {"Set File Name", setFileName},
        {"Set Unit Size", setUnitSize},
        {"Load Into Memory", loadMemory},
        {"Toggle Display Mode", toggleDisplay},
        {"Memory Display", memoryDisplay},
        {"Save Into File", saveIntoFile},
        {"Memory Modify",memoryModify},
        {"Quit", quit},
        {NULL, NULL}
      };
    
   int numOfItems = sizeof(menu) / sizeof(menu[0]) - 1;

    while (1) {

        if (s.debug_mode) {
        fprintf(stderr, "Debug info: unit_size=%d, file_name='%s', mem_count=%u\n",
                s.unit_size, s.file_name, s.mem_count);
        }

        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d) %s\n", i, menu[i].name);
        }
       
        printf("Choose Action\n");
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        } else {
            int option = atoi(input);
            if (option >= 0 && option < numOfItems) {
                menu[option].fun(&s);
            } else {
                printf("Not within bounds\n");
            }
        }
    }

    return 0;
}