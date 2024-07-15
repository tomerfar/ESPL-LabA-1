#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "elf.h" 

#define PAGE_SIZE 4096

extern int startup(int argc, char **argv, void (*start)());
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

    // flags
    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x ",
           type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, 
           phdr->p_filesz, phdr->p_memsz);

    // flags
    printf("%c%c%c ",
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ');

    printf("0x%x ", phdr->p_align);
    printf("prot=0x%x ", prot);
    printf("flags=0x%x\n", MAP_PRIVATE | MAP_FIXED);
}

void print_phdr_info(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, phdr);
}

// loading program headers into memory
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

            void *addr = (void *)(phdr[i].p_vaddr & ~(PAGE_SIZE - 1));

            size_t aligned_offset = phdr[i].p_vaddr & (PAGE_SIZE - 1);
            size_t adjusted_memsz = memsz + aligned_offset;

            void *mem = mmap(addr, adjusted_memsz, flags, MAP_PRIVATE, fd, offset - aligned_offset);
            if (mem == MAP_FAILED) {
                perror("mmap failed");
                exit(EXIT_FAILURE);
            }

            //print information
            print_phdr(&phdr[i], i);
        }
    }
}


//Main Function
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <executable>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // open elf
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    // reading elf table to get the elf offset
    Elf32_Ehdr elf_header;
    if (read(fd, &elf_header, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("read ELF header failed");
        close(fd);
        return EXIT_FAILURE;
    }

    Elf32_Phdr *phdr = malloc(elf_header.e_phentsize * elf_header.e_phnum);
    if (!phdr) {
        perror("malloc failed");
        close(fd);
        return EXIT_FAILURE;
    }

    lseek(fd, elf_header.e_phoff, SEEK_SET);
    if (read(fd, phdr, elf_header.e_phentsize * elf_header.e_phnum) != elf_header.e_phentsize * elf_header.e_phnum) {
        perror("read program headers failed");
        free(phdr);
        close(fd);
        return EXIT_FAILURE;
    }

    load_phdr(phdr, fd);

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

    // call startup with the entry point of the executable and args
    startup(argc - 1, args, (void *)(elf_header.e_entry));

    // freeing memory
    free(args);
    free(phdr);
    close(fd);

    return EXIT_SUCCESS;
}