#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *program_header = (Elf32_Phdr *)(map_start + elf_header->e_phoff);
    printf("Type     Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align  prot  flags\n");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < elf_header->e_phnum; i++) {
        func(&program_header[i], i);
    }
    return 0;
}

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

    // Mapping flags for mmap
    int flags;
    if (phdr->p_type == PT_DYNAMIC || phdr->p_type == PT_INTERP) {
        flags = MAP_SHARED;
    } else {
        flags = MAP_PRIVATE;
    }

    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x ",
           type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr, 
           phdr->p_filesz, phdr->p_memsz);

    // Flags
    printf("%c%c%c ",
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ');

    printf("0x%x ", phdr->p_align);

    // Protection and mapping flags for mmap
    printf("prot=0x%x flags=0x%x\n", prot, flags);
}

// void print_phdr(Elf32_Phdr *phdr, int index) {
//     printf("Program header number %d at address %p\n", index, (void *)phdr);
// }

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    void *map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    foreach_phdr(map_start, print_phdr, 0);

    munmap(map_start, st.st_size);
    return 0;
}