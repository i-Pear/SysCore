#include "elf_loader.h"

void *elf_read(void **source, int size) {
    void *res = k_malloc(size);
    memcpy(res, *source, size);
    *source += size;
    return res;
}

void *load_elf(const char *_elf_data, int size) {
    // check magic number
    if (_elf_data[0] != 0x7f || _elf_data[1] != 0x45 || _elf_data[2] != 0x4c || _elf_data[3] != 0x46) {
        printf("[ELF LOADER] Invalid ELF File! \n");
        return 0;
    }
    void *elf_start = (void *) _elf_data;

    // read elf header
    printf("[ELF LOADER] elf size = %d\n", size);
    /**
     * typedef struct {
     * unsigned char	e_ident[EI_NIDENT];
     * Elf64_Half	e_type;
     * Elf64_Half	e_machine;
     * Elf64_Word	e_version;
     * Elf64_Addr	e_entry;
     * Elf64_Off	e_phoff;
     * Elf64_Off	e_shoff;
     * Elf64_Word	e_flags;
     * Elf64_Half	e_ehsize;
     * Elf64_Half	e_phentsize;
     * Elf64_Half	e_phnum;
     * Elf64_Half	e_shentsize;
     * Elf64_Half	e_shnum;
     * Elf64_Half	e_shstrndx;
     * } Elf64_Ehdr;
     */

    // Elf64_Ehdr *header = elf_read(&elf_start, sizeof(Elf64_Ehdr));
    Elf64_Ehdr *Ehdr = elf_start;
    if (Ehdr->e_type == 2) {
        printf("[ELF LOADER] This a Executable file. OK.\n");
    }
    printf("[ELF LOADER] ELF entry: %x\n", Ehdr->e_entry);

//    char *exec = k_malloc(size);
    char *exec = (char *) alloc_page();
    for (int i = 1; i < 100; i++)alloc_page();

    Elf64_Phdr *phdr = (Elf64_Phdr *) (elf_start + Ehdr->e_phoff);
    Elf64_Shdr *shdr = (Elf64_Shdr *) (elf_start + Ehdr->e_shoff);
    Elf64_Sym *syms = NULL;
    char *strings = NULL;
    void *entry = NULL;

    for (int i = 0; i < Ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)continue;
        if (phdr[i].p_filesz == 0)continue;

        printf("[ELF LOADER] Copying Segment of size %d\n", phdr[i].p_filesz);

        char *start = elf_start + phdr[i].p_offset;
        char *target_addr = phdr[i].p_vaddr + exec;
        printf("[ELF LOADER] p_vaddr: 0x%x  exec: 0x%x\n", phdr[i].p_vaddr, exec);
        printf("[ELF LOADER] Start memcpy! from 0x%x to 0x%x \n", start, target_addr);
        entry=target_addr;

        memcpy(target_addr, start, phdr[i].p_filesz);
    }

    printf("[ELF LOADER] entry= %x\n",entry);

    return entry;
}
