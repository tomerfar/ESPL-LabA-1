// #include <stdio.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <elf.h>
// #include <sys/user.h>
// #include <string.h>



// void load_phdr(Elf32_Phdr *phdr, int elf_fd) {
//     if (phdr->p_type != PT_LOAD) {
//         printf("Skipping non-LOAD program header\n");
//         return;
//     }

//     // Calculate base address where segment should be loaded
//     void *load_addr = (void *)(phdr->p_vaddr & ~(PAGE_SIZE - 1));

//     // Map the segment into memory
//     int prot = PROT_READ | PROT_WRITE;
//     if (phdr->p_flags & PF_X)
//         prot |= PROT_EXEC;

//     // Map the segment into memory
//     void *mem = mmap(load_addr, phdr->p_memsz, prot, MAP_PRIVATE | MAP_FIXED, elf_fd, phdr->p_offset);
//     if (mem == MAP_FAILED) {
//         printf("error line 66");
//         perror("mmap");
//         return;
//     }

//     // Print information about the loaded segment
//     printf("Loaded program header:\n");
//     print_phdr(phdr, 0);
// }


// void load_elf(const char *filename) {
//     int fd = open(filename, O_RDONLY);
//     if (fd < 0) {
//         perror("open");
//         return;
//     }

//     // Get file size
//     struct stat st;
//     if (fstat(fd, &st) != 0) {
//         perror("fstat");
//         close(fd);
//         return;
//     }

//     // Map the ELF file into memory
//     void *map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
//     if (map_start == MAP_FAILED) {
//         printf("error in mmap");
//         perror("mmap");
//         close(fd);
//         return;
//     }

//     close(fd);

//     // Iterate over program headers and load into memory
//     Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
//     Elf32_Phdr *program_header = (Elf32_Phdr *)((char *)map_start + elf_header->e_phoff);

//     for (int i = 0; i < elf_header->e_phnum; i++) {
//         load_phdr(&program_header[i], fd);
//     }

//     // Find entry point and execute
//     void (*entry_point)() = (void (*)())(elf_header->e_entry);
//     entry_point();

//     // Unmap the mapped memory
//     munmap(map_start, st.st_size);
// }

// int main(int argc, char *argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
//         return 1;
//     }

//     const char *filename = argv[1];
//     load_elf(filename);

//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "elf.h"  // Include your ELF header file here

#define PAGE_SIZE 4096

extern int startup(int argc, char **argv, void (*start)());
// Function to print program header information
extern int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);

void print_phdr(Elf32_Phdr *phdr, int index) {
    const char *type;
    switch (phdr->p_type) {
        case PT_NULL: type = "NULL"; break;
        case PT_LOAD: type = "LOAD"; break;
        case PT_DYNAMIC: type = "DYNAMIC"; break;
        case PT_INTERP: type = "INTERP"; break;
        case PT_NOTE: type = "NOTE"; break;
        case PT_SHLIB: type = "SHLIB"; break;
        case PT_PHDR: type = "PHDR"; break;
        case PT_TLS: type = "TLS"; break;
        default: type = "UNKNOWN"; break;
    }

    // Protection flags for mmap
    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x ",
           type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, 
           phdr->p_filesz, phdr->p_memsz);

    // Flags
    printf("%c%c%c ",
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ');

    printf("0x%x ", phdr->p_align);

    // Protection flags for mmap
    printf("prot=0x%x ", prot);

    // Additional flags (optional)
    printf("flags=0x%x\n", MAP_PRIVATE | MAP_FIXED);

    // Note: MAP_FIXED is used to ensure the segment is mapped at the specified address
}

void print_phdr_info(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, phdr);
}

// Function to load program headers into memory
void load_phdr(Elf32_Phdr *phdr, int fd) {
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        perror("fstat failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < file_stat.st_size / sizeof(Elf32_Phdr); ++i) {
        if (phdr[i].p_type == PT_LOAD) {
            off_t offset = phdr[i].p_offset;
            size_t filesz = phdr[i].p_filesz;
            size_t memsz = phdr[i].p_memsz;
            int flags = 0;

            if (phdr[i].p_flags & PF_R)
                flags |= PROT_READ;
            if (phdr[i].p_flags & PF_W)
                flags |= PROT_WRITE;
            if (phdr[i].p_flags & PF_X)
                flags |= PROT_EXEC;

            // Ensure the virtual address is page-aligned
            void *addr = (void *)(phdr[i].p_vaddr & ~(PAGE_SIZE - 1));

            // Adjust the memory size to include the offset within the page
            size_t aligned_offset = phdr[i].p_vaddr & (PAGE_SIZE - 1);
            size_t adjusted_memsz = memsz + aligned_offset;

            void *mem = mmap(addr, adjusted_memsz, flags, MAP_PRIVATE, fd, offset - aligned_offset);
            if (mem == MAP_FAILED) {
                perror("mmap failed");
                exit(EXIT_FAILURE);
            }

            // Print information about the mapped program header
            print_phdr(&phdr[i], i);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <executable>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the ELF executable
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    // Read ELF header to get program header table offset
    Elf32_Ehdr elf_header;
    if (read(fd, &elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read ELF header failed");
        close(fd);
        return EXIT_FAILURE;
    }

    // Allocate memory for program headers
    Elf32_Phdr *phdr = malloc(elf_header.e_phentsize * elf_header.e_phnum);
    if (!phdr) {
        perror("malloc failed");
        close(fd);
        return EXIT_FAILURE;
    }

    // Read program headers
    lseek(fd, elf_header.e_phoff, SEEK_SET);
    if (read(fd, phdr, elf_header.e_phentsize * elf_header.e_phnum) != elf_header.e_phentsize * elf_header.e_phnum) {
        perror("read program headers failed");
        free(phdr);
        close(fd);
        return EXIT_FAILURE;
    }

    // Load program headers into memory
    load_phdr(phdr, fd);

    // Prepare arguments for the loaded program
    char **args = malloc(argc * sizeof(char *));
    if (!args) {
        perror("malloc failed");
        free(phdr);
        close(fd);
        return EXIT_FAILURE;
    }
    
    // Copy argv[1] onwards into args
    for (int i = 1; i < argc; i++) {
        args[i - 1] = argv[i];
    }
    args[argc - 1] = NULL;  // NULL-terminate the args array

    // Call startup with the entry point of the executable and args
    startup(argc - 1, args, (void *)(elf_header.e_entry));

    // Cleanup allocated memory and close file descriptor
    free(args);
    free(phdr);
    close(fd);

    return EXIT_SUCCESS;
}