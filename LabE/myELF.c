#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

// Global variables
typedef struct {
    int debug_mode;
    int fd1;
    int fd2;
    void *map_start1;
    void *map_start2;
    size_t file_size1;
    size_t file_size2;
    char file_name1[256];
    char file_name2[256];
} state;

// init
state s = {0, -1, -1, NULL, NULL, 0, 0};

// declarations
void toggle_debug_mode(state* s);
void examine_elf_file(state* s);
void print_section_names(state* s);
void print_symbols(state* s);
void check_files_for_merge(state* s);
void merge_elf_files(state* s);
void quit(state* s);


struct menu_option {
    char *name;
    void (*func)();
};

struct menu_option menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Examine ELF File", examine_elf_file},
    {"Print Section Names", print_section_names},
    {"Print Symbols", print_symbols},
    {"Check Files for Merge", check_files_for_merge},
    {"Merge ELF Files", merge_elf_files},
    {"Quit", quit},
};


void copy_char_array(char arr1[], char arr2[]) {
    for (int i = 0; i < 256 && arr2[i] != '\0'; i++) {
        arr1[i] = arr2[i];
    }
}


void toggle_debug_mode(state* s) {
    s->debug_mode = !s->debug_mode;
    printf("Debug mode %s\n", s->debug_mode ? "on" : "off");
}

void examine_elf_file(state* s) {
    if (s->fd1 != -1 && s->fd2 != -1) {
        printf("Two ELF files are already open. Cannot open more files.\n");
        return;
    }

    printf("Enter ELF file name: ");
    char file_name[256];
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; // for removing newline char

    int fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        perror("Error: Failed to open file");
        return;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error: Failed to get file size");
        close(fd);
        return;
    }

    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error: Failed to map file");
        close(fd);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    if (strncmp((char*)header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Error: Not an ELF file\n");
        munmap(map_start, file_size);
        close(fd);
        return;
    }

    printf("\n");
    printf("Magic: %c%c%c\n", header->e_ident[1], header->e_ident[2], header->e_ident[3]);
    printf("Data:%s\n", header->e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" : "Unknown");
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
        copy_char_array(s->file_name1, file_name);
    } else {
        s->fd2 = fd;
        s->map_start2 = map_start;
        s->file_size2 = file_size;
        copy_char_array(s->file_name2, file_name);
    }
}

// Converting section types into readable words
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

void print_section_names(state* s) {
    if (s->fd1 == -1 && s->fd2 == -1) {
        printf("Error: No ELF files opened. Please use 'Examine ELF File' first.\n");
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

        if (s->debug_mode) { //prints important information in case were in debug mode
        printf("Debug: ELF header details:\n");
        printf("  e_shoff: %x\n", header->e_shoff);
        printf("  e_shnum: %d\n", header->e_shnum);
        printf("  e_shstrndx: %d\n", header->e_shstrndx);
        printf("Debug: shstrtab details:\n");
        printf("  shstrtab_offset: %x\n", shstrtab_header->sh_offset);
        printf("  shstrtab_size: %x\n", shstrtab_header->sh_size);
        }

        printf("[index] section_name             section_address section_offset section_size section_type\n");

        for (int i = 0; i < header->e_shnum; i++) {
            printf("[%2d] %-24s 0x%08x      0x%06x         0x%06x       %s\n",
                i,
                &shstrtab[section_headers[i].sh_name],
                section_headers[i].sh_addr,
                section_headers[i].sh_offset,
                section_headers[i].sh_size,
                get_section_type(section_headers[i].sh_type));
        }
    }
}
// char* getSectionName(Elf32_Ehdr* elf_header, int index) {
//     Elf32_Shdr *section_header = (Elf32_Shdr *)((char *)elf_header + elf_header->e_shoff);
//     if (index >= 0 && index < elf_header->e_shnum) {
//         char *strtab = (char *)elf_header + section_header[elf_header->e_shstrndx].sh_offset;
//         return strtab + section_header[index].sh_name;
//     }
//     return "N/A";
// }

const char *get_section_name(Elf32_Ehdr *header, int index) {
    if (index >= 0 && index < header->e_shnum) {
        Elf32_Shdr *section_headers = (Elf32_Shdr *)((char *)header + header->e_shoff);
        Elf32_Shdr *shstrtab_header = &section_headers[header->e_shstrndx];
        char *shstrtab = (char *)header + shstrtab_header->sh_offset;
        return shstrtab + section_headers[index].sh_name;
    }
    return "Unaviliable";
}


void print_symbols(state* s) {
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
        Elf32_Shdr *section_headers = (Elf32_Shdr *)((char *)header + header->e_shoff);
        Elf32_Shdr *symtab_header = NULL;
        Elf32_Shdr *strtab_header = NULL;
        const char *strtab = NULL;

        for (int i = 0; i < header->e_shnum; i++) {
            if (section_headers[i].sh_type == SHT_SYMTAB || section_headers[i].sh_type == SHT_DYNSYM) {
                symtab_header = &section_headers[i];
                strtab_header = &section_headers[section_headers[i].sh_link];
                break;
            }
        }

        if (!symtab_header || !strtab_header) {
            printf("No symbol table found in ELF file.\n");
            continue;
        }

        int symbol_count = symtab_header->sh_size / sizeof(Elf32_Sym);
        Elf32_Sym *symtab = (Elf32_Sym *)((char *)header + symtab_header->sh_offset);
        strtab = (const char *)((char *)header + strtab_header->sh_offset);

        if (s->debug_mode) {
            printf("Debug: Symbol table size: %d\n", symtab_header->sh_size);
            printf("Debug: Number of symbols: %d\n", symbol_count);
        }

        printf("\nFile: %s\n", (file_index == 0) ? s->file_name1 : s->file_name2);
        printf("[index] value section_index section_name symbol_name\n");

        // printing the symbols info
        for (int i = 0; i < symbol_count; i++) {
            const char *section_name = get_section_name(header, symtab[i].st_shndx);
            printf("[%2d] 0x%08x %d %s %s\n",
                   i,
                   symtab[i].st_value,
                   symtab[i].st_shndx,
                   section_name,
                   strtab + symtab[i].st_name);
        }
    }
}


// checks whether we can merge the current loaded files
void check_files_for_merge(state* s) {
    if (s->fd1 == -1 || s->fd2 == -1) {
        printf("Two ELF files must be opened for merging.\n");
        return;
    }

    Elf32_Ehdr *header1 = (Elf32_Ehdr *)s->map_start1;
    Elf32_Ehdr *header2 = (Elf32_Ehdr *)s->map_start2;
    Elf32_Shdr *section_headers1 = (Elf32_Shdr *)((char *)header1 + header1->e_shoff);
    Elf32_Shdr *section_headers2 = (Elf32_Shdr *)((char *)header2 + header2->e_shoff);
    Elf32_Shdr *symtab_header1 = NULL;
    Elf32_Shdr *symtab_header2 = NULL;

    // symbols in the 1st elf file
    for (int i = 0; i < header1->e_shnum; i++) {
        if (section_headers1[i].sh_type == SHT_SYMTAB || section_headers1[i].sh_type == SHT_DYNSYM) {
            if (symtab_header1) {
                printf("Multiple symbol tables found in the first ELF file. Feature not supported.\n");
                return;
            }
            symtab_header1 = &section_headers1[i];
        }
    }

    if (!symtab_header1) {
        printf("No symbol table found in the first ELF file. Feature not supported.\n");
        return;
    }

    // symbols in the 2nd elf file
    for (int i = 0; i < header2->e_shnum; i++) {
        if (section_headers2[i].sh_type == SHT_SYMTAB || section_headers2[i].sh_type == SHT_DYNSYM) {
            if (symtab_header2) {
                printf("Multiple symbol tables found in the second ELF file. Feature not supported.\n");
                return;
            }
            symtab_header2 = &section_headers2[i];
        }
    }

    if (!symtab_header2) {
        printf("No symbol table found in the second ELF file. Feature not supported.\n");
        return;
    }

    // Retrieve symbol table data
    Elf32_Sym *symtab1 = (Elf32_Sym *)((char *)header1 + symtab_header1->sh_offset);
    Elf32_Sym *symtab2 = (Elf32_Sym *)((char *)header2 + symtab_header2->sh_offset);
    const char *strtab1 = (const char *)((char *)header1 + section_headers1[symtab_header1->sh_link].sh_offset);
    const char *strtab2 = (const char *)((char *)header2 + section_headers2[symtab_header2->sh_link].sh_offset);
    int symbol_count1 = symtab_header1->sh_size / sizeof(Elf32_Sym);
    int symbol_count2 = symtab_header2->sh_size / sizeof(Elf32_Sym);

    // Check each symbol in the first symbol table
    for (int i = 1; i < symbol_count1; i++) { // Skip the first symbol (dummy null symbol)
        Elf32_Sym *sym1 = &symtab1[i];
        const char *sym1_name = strtab1 + sym1->st_name;

        if (ELF32_ST_BIND(sym1->st_info) == STB_GLOBAL) {
            // Check if symbol is UNDEFINED in SYMTAB2
            if (sym1->st_shndx == SHN_UNDEF) {
                int found = 0;
                for (int j = 1; j < symbol_count2; j++) { // Skip the first symbol (dummy null symbol)
                    Elf32_Sym *sym2 = &symtab2[j];
                    const char *sym2_name = strtab2 + sym2->st_name;
                    if (strcmp(sym1_name, sym2_name) == 0 && sym2->st_shndx != SHN_UNDEF) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("Symbol %s undefined\n", sym1_name);
                }
            }
            // Check if symbol is DEFINED in SYMTAB2
            else {
                int found = 0;
                for (int j = 1; j < symbol_count2; j++) { // Skip the first symbol (dummy null symbol)
                    Elf32_Sym *sym2 = &symtab2[j];
                    const char *sym2_name = strtab2 + sym2->st_name;
                    if (strcmp(sym1_name, sym2_name) == 0 && sym2->st_shndx != SHN_UNDEF) {
                        found = 1;
                        printf("Symbol %s multiply defined\n", sym1_name);
                        break;
                    }
                }
            }
        }
    }
}

void merge_elf_files(state* s) {
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

int main(int argc, char **argv) {
    while (1) {
        printf("Choose action:\n");
        for (int i = 0; i < sizeof(menu) / sizeof(menu[0]); i++) {
            printf("%d-%s\n", i, menu[i].name);
        }

        int choice;
        scanf("%d", &choice);

        getchar(); // Clear the newline character from the input buffer

        if (choice >= 0 && choice < sizeof(menu) / sizeof(menu[0])) {
            menu[choice].func(&s);
        } else {
            printf("Invalid option\n");
        }
    }

    return 0;
}
