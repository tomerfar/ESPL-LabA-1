#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

// Global variables
int debug_mode = 0;
int fd1 = -1, fd2 = -1;
void *map_start1 = NULL, *map_start2 = NULL;
struct stat fd_stat1, fd_stat2;

// Function prototypes
void toggle_debug_mode();
void examine_elf_file();
void print_section_names();
void print_symbols();
void check_files_for_merge();
void merge_elf_files();
void quit();

// Menu structure
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

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    printf("Debug mode %s\n", debug_mode ? "on" : "off");
}

void examine_elf_file() {
    char filename[100];
    printf("Enter ELF file name: ");
    scanf("%s", filename);

    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    struct stat fd_stat;
    if (fstat(fd, &fd_stat) != 0) {
        perror("Error getting file stat");
        close(fd);
        return;
    }

    void *map_start = mmap(NULL, fd_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *) map_start;

    if (strncmp((char *) header->e_ident, "\x7f""ELF", 4) != 0) {
        printf("Not an ELF file\n");
        munmap(map_start, fd_stat.st_size);
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

    // Assign to global variables if necessary
    if (fd1 == -1) {
        fd1 = fd;
        map_start1 = map_start;
        fd_stat1 = fd_stat;
    } else if (fd2 == -1) {
        fd2 = fd;
        map_start2 = map_start;
        fd_stat2 = fd_stat;
    } else {
        printf("Already two ELF files are open. Close one to open a new file.\n");
        munmap(map_start, fd_stat.st_size);
        close(fd);
    }
}

// const char *get_section_type_name(uint32_t sh_type) {
//     switch (sh_type) {
//         case SHT_NULL:     return "NULL";
//         case SHT_PROGBITS: return "PROGBITS";
//         case SHT_SYMTAB:   return "SYMTAB";
//         case SHT_STRTAB:   return "STRTAB";
//         case SHT_RELA:     return "RELA";
//         case SHT_HASH:     return "HASH";
//         case SHT_DYNAMIC:  return "DYNAMIC";
//         case SHT_NOTE:     return "NOTE";
//         case SHT_NOBITS:   return "NOBITS";
//         case SHT_REL:      return "REL";
//         case SHT_SHLIB:    return "SHLIB";
//         case SHT_DYNSYM:   return "DYNSYM";
//         default:           return "UNKNOWN";
//     }
// }


// void print_section_names_for_file(int fd, void *map_start, struct stat fd_stat) {
//     Elf32_Ehdr *header = (Elf32_Ehdr *) map_start;

//     if (header->e_shoff > fd_stat.st_size) {
//         printf("Invalid section header offset\n");
//         return;
//     }

//     Elf32_Shdr *section_headers = (Elf32_Shdr *) (map_start + header->e_shoff);

//     if (header->e_shstrndx == SHN_UNDEF) {
//         printf("No section name string table index\n");
//         return;
//     }

//     if (header->e_shstrndx >= header->e_shnum) {
//         printf("Invalid section header string table index\n");
//         return;
//     }

//     Elf32_Shdr *shstrtab_header = &section_headers[header->e_shstrndx];

//     if (shstrtab_header->sh_offset > fd_stat.st_size) {
//         printf("Invalid section header string table offset\n");
//         return;
//     }

//     const char *shstrtab = (const char *) (map_start + shstrtab_header->sh_offset);

//     if (debug_mode) {
//         printf("Debug: ELF header details:\n");
//         printf("  e_shoff: %x\n", header->e_shoff);
//         printf("  e_shnum: %d\n", header->e_shnum);
//         printf("  e_shstrndx: %d\n", header->e_shstrndx);
//         printf("Debug: shstrtab details:\n");
//         printf("  shstrtab_offset: %x\n", shstrtab_header->sh_offset);
//         printf("  shstrtab_size: %x\n", shstrtab_header->sh_size);
//     }

//     printf("File: %s\n", (fd == fd1) ? "File 1" : "File 2");

//     for (int i = 0; i < header->e_shnum; i++) {
//         Elf32_Shdr *sh = &section_headers[i];
//         const char *section_name = shstrtab + sh->sh_name;

//         // Check for invalid section name pointer
//         if ((const char *) section_name >= (const char *) map_start + fd_stat.st_size || (const char *) section_name < (const char *) map_start) {
//             printf("Invalid section name pointer at index %d: %p\n", i, section_name);
//             continue;
//         }

//         printf("[%2d] %-20s %08x %06x %06x %s\n",
//                i,
//                section_name,
//                sh->sh_addr,
//                sh->sh_offset,
//                sh->sh_size,
//                get_section_type_name(sh->sh_type));
//     }
// }

// void print_section_names() {
//     if (fd1 == -1 && fd2 == -1) {
//         printf("No ELF files currently opened.\n");
//         return;
//     }

//     if (debug_mode) {
//         printf("Debug: fd1 = %d, fd2 = %d\n", fd1, fd2);
//         printf("Debug: map_start1 = %p, map_start2 = %p\n", map_start1, map_start2);
//     }

//     if (fd1 != -1) {
//         print_section_names_for_file(fd1, map_start1, fd_stat1);
//     }
//     if (fd2 != -1) {
//         print_section_names_for_file(fd2, map_start2, fd_stat2);
//     }
// }

void print_section_names() {
    if (fd1 == -1 && fd2 == -1) {
         printf("No ELF files currently opened.\n");
         return;
     }
    
    if(fd1 != -1)
    {
        printf("File1: ELF-file-name\n");
        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start1;
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_start1 + elf_header->e_shoff);
        char *section_names = (char *)((char *)map_start1 + section_header_table[elf_header->e_shstrndx].sh_offset);

        printf("[index] section_name section_address section_offset section_size section_type\n");

        for (int j = 0; j < elf_header->e_shnum; j++) {
            printf("[%d] %s 0x%08x 0x%08x %08x 0x%x\n", j,
                    &section_names[section_header_table[j].sh_name],
                    section_header_table[j].sh_addr,
                    section_header_table[j].sh_offset,
                    section_header_table[j].sh_size,
                    section_header_table[j].sh_type);
        }
    }
    if(fd2 != -1)
    {
        printf("File2: ELF-file-name\n");
        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start2;
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_start2 + elf_header->e_shoff);
        char *section_names = (char *)((char *)map_start2 + section_header_table[elf_header->e_shstrndx].sh_offset);

        printf("[index] section_name section_address section_offset section_size section_type\n");

        for (int j = 0; j < elf_header->e_shnum; j++) {
            printf("[%d] %s 0x%08x 0x%08x %08x 0x%x\n", j,
                    &section_names[section_header_table[j].sh_name],
                    section_header_table[j].sh_addr,
                    section_header_table[j].sh_offset,
                    section_header_table[j].sh_size,
                    section_header_table[j].sh_type);
        }
    }
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

void quit() {
    if (fd1 != -1) {
        munmap(map_start1, fd_stat1.st_size);
        close(fd1);
    }
    if (fd2 != -1) {
        munmap(map_start2, fd_stat2.st_size);
        close(fd2);
    }
    printf("Exiting...\n");
    exit(0);
}

int main() {
    while (1) {
        printf("Choose action:\n");
        for (int i = 0; i < sizeof(menu) / sizeof(menu[0]); i++) {
            printf("%d-%s\n", i, menu[i].name);
        }

        int choice;
        scanf("%d", &choice);

        if (choice >= 0 && choice < sizeof(menu) / sizeof(menu[0])) {
            menu[choice].func();
        } else {
            printf("Invalid option\n");
        }
    }

    return 0;
}
