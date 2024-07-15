#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>

#define MAX_FILES 2

struct option {
    int action;
    char *description;
};

int debug_mode = 0;
int current_files = 0;

struct option options[] = {
    {0, "Toggle Debug Mode"},
    {1, "Examine ELF File"},
    {2, "Print Section Names"},
    {3, "Print Symbols"},
    {4, "Check Files for Merge"},
    {5, "Merge ELF Files"},
    {6, "Quit"}
};

int file_descriptors[MAX_FILES] = {-1, -1};
void *map_starts[MAX_FILES];

void toggle_debug_mode() {
    debug_mode = 1 - debug_mode;
    printf("Debug Mode %s\n", debug_mode ? "Enabled" : "Disabled");
}

void print_error(char *message) {
    perror(message);
}

void unmap_and_close(int file_index) {
    if (file_descriptors[file_index] != -1) {
        if (munmap(map_starts[file_index], sizeof(Elf32_Ehdr)) == -1) {
            print_error("munmap");
        }
        if (close(file_descriptors[file_index]) == -1) {
            print_error("close");
        }
        file_descriptors[file_index] = -1;
    }
}

void examine_elf_file() {
    if (current_files >= MAX_FILES) {
        printf("Cannot handle more than %d ELF files\n", MAX_FILES);
        return;
    }

    char file_name[256];
    printf("Enter ELF file name: ");
    scanf("%s", file_name);

    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        print_error("open");
        return;
    }

    void *map_start = mmap(NULL, sizeof(Elf32_Ehdr), PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        print_error("mmap");
        close(fd);
        return;
    }

    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;

    // Verify ELF magic number
    if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Not a valid ELF file\n");
        unmap_and_close(current_files);
        return;
    }

    printf("Bytes 1,2,3 of the magic number (in ASCII): %c%c%c\n",
           elf_header->e_ident[EI_MAG1], elf_header->e_ident[EI_MAG2], elf_header->e_ident[EI_MAG3]);
    printf("Data encoding scheme: %s\n", elf_header->e_ident[EI_DATA] == ELFDATA2MSB ? "big-endian" : "little-endian");
    printf("Entry point: 0x%08x\n", elf_header->e_entry);
    printf("Section header table offset: %u\n", elf_header->e_shoff);
    printf("Number of section header entries: %u\n", elf_header->e_shnum);
    printf("Size of each section header entry: %u\n", elf_header->e_shentsize);
    printf("Program header table offset: %u\n", elf_header->e_phoff);
    printf("Number of program header entries: %u\n", elf_header->e_phnum);
    printf("Size of each program header entry: %u\n", elf_header->e_phentsize);

    // Store information for later use
    file_descriptors[current_files] = fd;
    map_starts[current_files] = map_start;
    current_files++;
}

void print_section_names() {
    if (current_files == 0) {
        printf("No ELF files opened yet.\n");
        return;
    }

    for (int i = 0; i < current_files; i++) {
        printf("File %d: ELF-file-name\n", i + 1);
        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_starts[i];
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_starts[i] + elf_header->e_shoff);
        char *section_names = (char *)((char *)map_starts[i] + section_header_table[elf_header->e_shstrndx].sh_offset);

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
    if (current_files == 0) {
        printf("No ELF files opened yet.\n");
        return;
    }

    for (int i = 0; i < current_files; i++) {
        printf("File %d: ELF-file-name\n", i + 1);
        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_starts[i];
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_starts[i] + elf_header->e_shoff);
        char *section_names = (char *)((char *)map_starts[i] + section_header_table[elf_header->e_shstrndx].sh_offset);

        // Find the symbol table section
        Elf32_Shdr *symtab_section = NULL;
        Elf32_Shdr *strtab_section = NULL;

        for (int j = 0; j < elf_header->e_shnum; j++) {
            if (section_header_table[j].sh_type == SHT_SYMTAB) {
                symtab_section = &section_header_table[j];
            } else if (section_header_table[j].sh_type == SHT_STRTAB) {
                strtab_section = &section_header_table[j];
            }
        }

        if (symtab_section == NULL || strtab_section == NULL) {
            printf("No symbol table found in the ELF file.\n");
            return;
        }

        // Retrieve the symbol table and string table
        Elf32_Sym *symbol_table = (Elf32_Sym *)((char *)map_starts[i] + symtab_section->sh_offset);
        char *strtab = (char *)((char *)map_starts[i] + strtab_section->sh_offset);

        if (symbol_table == NULL || strtab == NULL) {
            printf("Error accessing symbol or string table.\n");
            return;
        }

        printf("[index] value section_index section_name symbol_name\n");

        for (int k = 0; k < symtab_section->sh_size / sizeof(Elf32_Sym); k++) {
            // Ensure the section index is valid
            if (symbol_table[k].st_shndx < elf_header->e_shnum) {
                printf("[%d] 0x%08x %u %s %s\n", k,
                       symbol_table[k].st_value,
                       symbol_table[k].st_shndx,
                       &section_names[section_header_table[symbol_table[k].st_shndx].sh_name],
                       &strtab[symbol_table[k].st_name]);
            } else {
                printf("[%d] 0x%08x %u %s\n", k,
                       symbol_table[k].st_value,
                       symbol_table[k].st_shndx,
                       &strtab[symbol_table[k].st_name]);
            }
        }
    }
}
Elf32_Shdr *find_symbol_table(int file_index) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_starts[file_index];
    Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_starts[file_index] + elf_header->e_shoff);

    for (int j = 0; j < elf_header->e_shnum; j++) {
        if (section_header_table[j].sh_type == SHT_SYMTAB) {
            return &section_header_table[j];
        }
    }

    return NULL;
}

int get_strtab_offset(Elf32_Shdr *symtab_section) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_starts[0];
    Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_starts[0] + elf_header->e_shoff);
    return section_header_table[symtab_section->sh_link].sh_offset;
}

Elf32_Sym *find_symbol(Elf32_Sym *symbol_table, char *strtab, Elf32_Word symbol_name, Elf32_Shdr *symtab_section) {
    for (int k = 0; k < symtab_section->sh_size / sizeof(Elf32_Sym); k++) {
        if (symbol_table[k].st_name == symbol_name) {
            return &symbol_table[k];
        }
    }

    return NULL;
}
void check_files_for_merge() {
    int i=0;
    // Check if exactly two ELF files have been opened and mapped
    if (current_files != 2) {
        printf("Error: Two ELF files must be opened for merge.\n");
        return;
    }
        printf("File %d: ELF-file-name\n", i + 1);
        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_starts[i];
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((char *)map_starts[i] + elf_header->e_shoff);
        char *section_names = (char *)((char *)map_starts[i] + section_header_table[elf_header->e_shstrndx].sh_offset);

        // Find the symbol table section
        Elf32_Shdr *symtab_section = NULL;
        Elf32_Shdr *strtab_section = NULL;

        for (int j = 0; j < elf_header->e_shnum; j++) {
            if (section_header_table[j].sh_type == SHT_SYMTAB) {
                symtab_section = &section_header_table[j];
            } else if (section_header_table[j].sh_type == SHT_STRTAB) {
                strtab_section = &section_header_table[j];
            }
        }

        if (symtab_section == NULL || strtab_section == NULL) {
            printf("No symbol table found in the ELF file.\n");
            return;
        }

        // Retrieve the symbol table and string table
        Elf32_Sym *symbol_table = (Elf32_Sym *)((char *)map_starts[i] + symtab_section->sh_offset);
        char *strtab = (char *)((char *)map_starts[i] + strtab_section->sh_offset);

        if (symbol_table == NULL || strtab == NULL) {
            printf("Error accessing symbol or string table.\n");
            return;
        }

        printf("[index] value section_index section_name symbol_name\n");

        for (int k = 0; k < symtab_section->sh_size / sizeof(Elf32_Sym); k++) {
            // Ensure the section index is valid
            if (symbol_table[k].st_shndx < elf_header->e_shnum) {
                printf("[%d] 0x%08x %u %s %s\n", k,
                       symbol_table[k].st_value,
                       symbol_table[k].st_shndx,
                       &section_names[section_header_table[symbol_table[k].st_shndx].sh_name],
                       &strtab[symbol_table[k].st_name]);
            } else {
                printf("[%d] 0x%08x %u %s\n", k,
                       symbol_table[k].st_value,
                       symbol_table[k].st_shndx,
                       &strtab[symbol_table[k].st_name]);
            }
        }
    
}



void merge_elf_files() {
    printf("Not implemented yet\n");
}

void quit() {
    for (int i = 0; i < MAX_FILES; i++) {
        unmap_and_close(i);
    }
    exit(0);
}

int main() {
    while (1) {
        printf("\nChoose action:\n");
        for (int i = 0; i < sizeof(options) / sizeof(options[0]); i++) {
            printf("%d-%s\n", options[i].action, options[i].description);
        }

        int choice;
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                toggle_debug_mode();
                break;
            case 1:
                examine_elf_file();
                break;
            case 2:
                print_section_names();
                break;
            case 3:
                print_symbols();
                break;
            case 4:
                check_files_for_merge();
                break;
            case 5:
                merge_elf_files();
                break;
            case 6:
                quit();
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    }

    return 0;
}

