#include "elf_loader.h"
#include "../lib/stl/stl.h"
#include "memory/memory.h"

void load_elf(const char* _elf_data,int size,size_t* elf_page_base,size_t* elf_page_size,size_t* entry,Elf64_Off* e_phoff,int* phnum,Elf64_Phdr** kernel_phdr) {
    // check magic number
    if (_elf_data[0] != 0x7f || _elf_data[1] != 0x45 || _elf_data[2] != 0x4c || _elf_data[3] != 0x46) {
        printf("[ELF LOADER] Invalid ELF File! \n");
        shutdown();
        return;
    }
    const char *elf_start = _elf_data;
// #define DEBUG_ELF
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
    Elf64_Ehdr *Ehdr = (Elf64_Ehdr *) elf_start;
    if (Ehdr->e_type == 2) {
#ifdef DEBUG_ELF
        printf("[ELF LOADER] This a Executable file. OK.\n");
#endif
    }
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF entry: %x\n", Ehdr->e_entry);
#endif
    char *exec;
    *e_phoff=Ehdr->e_phoff;
    *phnum=Ehdr->e_phnum;

    Elf64_Phdr *phdr = (Elf64_Phdr *) (elf_start + Ehdr->e_phoff);
    int load_segment_count=0;
    size_t need_alloc_page=0;
    for (int i = 0; i < Ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)continue;
        if (phdr[i].p_filesz == 0)continue;
        load_segment_count++;
        need_alloc_page= max(need_alloc_page,phdr[i].p_vaddr+phdr[i].p_filesz);
    }
    exec = (char *) alloc_page(need_alloc_page);
    memset(exec,0,need_alloc_page);
    *elf_page_size=need_alloc_page;
    // printf("elf segment count = %d \n",load_segment_count);

    // copy kernel Phdr
    *kernel_phdr= (new Elf64_Phdr[Ehdr->e_phnum]);
    memcpy(*kernel_phdr,phdr, sizeof(Elf64_Phdr)*Ehdr->e_phnum);

    for (int i = 0; i < Ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)continue;
        if (phdr[i].p_filesz == 0)continue;

        const char *segment_start_addr = elf_start + phdr[i].p_offset;
        char *segment_target_addr = phdr[i].p_vaddr + exec;

        memcpy(segment_target_addr, segment_start_addr, phdr[i].p_filesz);
    }

    *elf_page_base=(size_t)exec;
    *entry=Ehdr->e_entry;
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF loaded successfully\n");
#endif
}
