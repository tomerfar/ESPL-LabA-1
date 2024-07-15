#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>

typedef struct {
    int debug_mode;    // Flag for debug mode
    int fd1;           // File descriptor for the first ELF file
    int fd2;           // File descriptor for the second ELF file
    void *map_start1;  // Memory-mapped start address for the first ELF file
    void *map_start2;  // Memory-mapped start address for the second ELF file
    size_t file_size1; // Size of the first ELF file
    size_t file_size2; // Size of the second ELF file
    char file_name1[256];
    char file_name2[256];
} state;

// Initialize the state structure
state s = {0, -1, -1, NULL, NULL, 0, 0};

void copyCharArr(char arr1[], char arr2[]) {
    for (int i = 0; i < 256 && arr2[i] != '\0'; i++) {
        arr1[i] = arr2[i];
    }
}




// Menu structure
struct {
    char *name;
    void (*func)(state*);
} menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Examine ELF File", examine_elf_file},
    {"Print Section Names", print_section_names},
    {"Print Symbols", print_symbols},
    {"Check Files for Merge", check_files_for_merge},
    {"Merge ELF Files", merge_elf_files},
    {"Quit", quit},
    {NULL, NULL}
};

int main() {
    while (1) {
        printf("\nChoose action:\n");
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%d-%s\n", i, menu[i].name);
        }

        printf("Option: ");
        char input[10];
        fgets(input, sizeof(input), stdin);
        int choice = atoi(input);

        if (choice >= 0 && choice <= 6) {
            menu[choice].func(&s);
        } else {
            printf("Invalid choice\n");
        }
    }

    return 0;
}


// Toggle debug mode
void toggle_debug_mode(state* s) {
    s->debug_mode = !s->debug_mode;
    printf("Debug mode %s\n", s->debug_mode ? "on" : "off");
}


// Examine an ELF file and print its header information
void examine_elf_file(state* s) {
    if (s->fd1 != -1 && s->fd2 != -1) {
        printf("Two ELF files are already open. Cannot open more files.\n");
        return;
    }

    printf("Enter ELF file name: ");
    char file_name[256];
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; // Remove newline character

    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file");
        return;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Failed to get file size");
        close(fd);
        return;
    }

    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Failed to map file");
        close(fd);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    if (strncmp((char*)header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Not an ELF file\n");
        munmap(map_start, file_size);
        close(fd);
        return;
    }

    printf("\n");
    printf("Magic: %c%c%c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    printf("Data: %d\n", header->e_ident[EI_DATA]);
    printf("Entry point: 0x%x\n", header->e_entry);
    printf("Section header offset: %d\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of section header: %d\n", header->e_shentsize);
    printf("Program header offset: %d\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of program header: %d\n", header->e_phentsize);

    if (s->fd1 == -1) {
        s->fd1 = fd;
        s->map_start1 = map_start;
        s->file_size1 = file_size;
        copyCharArr(s->file_name1, file_name);
    } else {
        s->fd2 = fd;
        s->map_start2 = map_start;
        s->file_size2 = file_size;
        copyCharArr(s->file_name2, file_name);
    }
}


// Function to convert section type to a human-readable string
const char *get_section_type(uint32_t sh_type) {
    switch (sh_type) {
        case SHT_NULL:          return "NULL";
        case SHT_PROGBITS:      return "PROGBITS";
        case SHT_SYMTAB:        return "SYMTAB";
        case SHT_STRTAB:        return "STRTAB";
        case SHT_RELA:          return "RELA";
        case SHT_HASH:          return "HASH";
        case SHT_DYNAMIC:       return "DYNAMIC";
        case SHT_NOTE:          return "NOTE";
        case SHT_NOBITS:        return "NOBITS";
        case SHT_REL:           return "REL";
        case SHT_SHLIB:         return "SHLIB";
        case SHT_DYNSYM:        return "DYNSYM";
        case SHT_INIT_ARRAY:    return "INIT_ARRAY";
        case SHT_FINI_ARRAY:    return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY: return "PREINIT_ARRAY";
        case SHT_GROUP:         return "GROUP";
        case SHT_SYMTAB_SHNDX:  return "SYMTAB_SHNDX";
        default:                return "UNKNOWN";
    }
}

// Print section names of the ELF files
void print_section_names(state* s) {
    if (s->fd1 == -1 && s->fd2 == -1) {
        printf("No ELF files opened. Please use 'Examine ELF File' first.\n");
        return;
    }

    for (int file_index = 0; file_index < 2; file_index++) {
        int fd = (file_index == 0) ? s->fd1 : s->fd2;
        void *map_start = (file_index == 0) ? s->map_start1 : s->map_start2;

        if (fd == -1) {
            continue;
        }

        Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
        Elf32_Shdr *section_headers = (Elf32_Shdr *)(map_start + header->e_shoff);
        Elf32_Shdr *shstrtab_header = &section_headers[header->e_shstrndx];
        const char *shstrtab = (const char *)(map_start + shstrtab_header->sh_offset);

        printf("\nFile: %s\n", (file_index == 0) ? s->file_name1 : s->file_name2);
        for (int i = 0; i < header->e_shnum; i++) {
            printf("[%2d] %s 0x%08x 0x%06x 0x%06x %s\n",
                   i,
                   &shstrtab[section_headers[i].sh_name],
                   section_headers[i].sh_addr,
                   section_headers[i].sh_offset,
                   section_headers[i].sh_size,
                   get_section_type(section_headers[i].sh_type));
        }
    }
}

char* getSectionName(Elf32_Ehdr* elf_header, int index) {
    Elf32_Shdr *section_header = (Elf32_Shdr *)((char *)elf_header + elf_header->e_shoff);
    if (index >= 0 && index < elf_header->e_shnum) {
        char *strtab = (char *)elf_header + section_header[elf_header->e_shstrndx].sh_offset;
        return strtab + section_header[index].sh_name;
    }
    return "N/A";
}


void print_symbols() {
    printf("Print Symbols: not implemented yet\n");
}

void check_files_for_merge() {
    printf("Check Files for Merge: not implemented yet\n");
}

void merge_elf_files() {
    printf("Merge ELF Files: not implemented yet\n");
}

void quit(state* s) {
    if (s->fd1 != -1) {
        munmap(s->map_start1, s->file_size1);
        close(s->fd1);
    }
    if (s->fd2 != -1) {
        munmap(s->map_start2, s->file_size2);
        close(s->fd2);
    }
    printf("Exiting...\n");
    exit(0);
}