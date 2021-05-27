#include "elf_loader.h"

void load_elf(const char* _elf_data,int size,size_t* elf_page_base,size_t* elf_page_size,size_t* entry) {
    // check magic number
    if (_elf_data[0] != 0x7f || _elf_data[1] != 0x45 || _elf_data[2] != 0x4c || _elf_data[3] != 0x46) {
        printf("[ELF LOADER] Invalid ELF File! \n");
        shutdown();
        return;
    }
    void *elf_start = (void *) _elf_data;

    // read elf header
#ifdef DEBUG_ELF
    printf("[ELF LOADER] elf size = %d\n", size);
#endif
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
#ifdef DEBUG_ELF
        printf("[ELF LOADER] This a Executable file. OK.\n");
#endif
    }
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF entry: %x\n", Ehdr->e_entry);
#endif
    char *exec = (char *) alloc_page(size);
    *elf_page_size=size;

    Elf64_Phdr *phdr = (Elf64_Phdr *) (elf_start + Ehdr->e_phoff);
    Elf64_Shdr *shdr = (Elf64_Shdr *) (elf_start + Ehdr->e_shoff);
    Elf64_Sym *syms = NULL;
    char *strings = NULL;

    for (int i = 0; i < Ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)continue;
        if (phdr[i].p_filesz == 0)continue;

//        printf("[ELF LOADER] Copying Segment of size %d\n", phdr[i].p_filesz);

        char *start = elf_start + phdr[i].p_offset;
        char *target_addr = phdr[i].p_vaddr + exec;
//        printf("[ELF LOADER] p_vaddr: 0x%x  exec: 0x%x\n", phdr[i].p_vaddr, exec);
//        printf("[ELF LOADER] Start memcpy! from 0x%x to 0x%x \n", start, target_addr);

        memcpy(target_addr, start, phdr[i].p_filesz);
    }

    *elf_page_base=(size_t)exec;
    *entry=Ehdr->e_entry;
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF loaded successfully\n");
#endif
}
