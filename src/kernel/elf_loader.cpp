#include "elf_loader.h"
#include "../lib/stl/stl.h"
#include "memory/memory.h"

void load_elf(FIL* elf_file,size_t* elf_page_base,size_t* elf_page_size,size_t* entry,Elf64_Off* e_phoff,int* phnum,Elf64_Phdr** kernel_phdr) {
    uint32_t read_bytes;
    auto* Ehdr=new Elf64_Ehdr();
    f_lseek(elf_file,0);
    f_read(elf_file,Ehdr, sizeof(Elf64_Ehdr),&read_bytes);
    // check magic number
    if (Ehdr->e_ident[0] != 0x7f || Ehdr->e_ident[1] != 0x45 || Ehdr->e_ident[2] != 0x4c || Ehdr->e_ident[3] != 0x46) {
        printf("[ELF LOADER] Invalid ELF File! \n");
        shutdown();
        return;
    }
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
    if (Ehdr->e_type == 2) {
#ifdef DEBUG_ELF
        printf("[ELF LOADER] This is a Executable file. OK.\n");
#endif
    }else{
        panic("[ELF LOADER] This is not a Executable file.")
    }
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF entry: %x\n", Ehdr->e_entry);
#endif
    char *exec;
    *e_phoff=Ehdr->e_phoff;
    *phnum=Ehdr->e_phnum;

    // copy kernel Phdr
    *kernel_phdr= new Elf64_Phdr[Ehdr->e_phnum];
    f_lseek(elf_file,Ehdr->e_phoff);
    f_read(elf_file,*kernel_phdr, sizeof(Elf64_Phdr)*Ehdr->e_phnum,&read_bytes);

    Elf64_Phdr *phdr = *kernel_phdr;
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

    for (int i = 0; i < Ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)continue;
        if (phdr[i].p_filesz == 0)continue;

        f_lseek(elf_file,phdr[i].p_offset);
        char *segment_target_addr = phdr[i].p_vaddr + exec;
        f_read(elf_file,segment_target_addr,phdr[i].p_filesz,&read_bytes);
    }

    *elf_page_base=(size_t)exec;
    *entry=Ehdr->e_entry;
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF loaded successfully\n");
#endif
}
