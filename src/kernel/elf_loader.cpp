#include "elf_loader.h"
#include "../lib/stl/stl.h"
#include "memory/memory.h"
#include "memory/Heap.h"

void load_elf(FIL* elf_file,Elf_Control* elf_control,size_t* entry,Elf64_Off* e_phoff,int* phnum,Elf64_Phdr** kernel_phdr) {
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
    // printf("elf segment count = %d \n",load_segment_count);

    elf_control->init_segments();
    for (int i = 0; i < Ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD)continue;
        if (phdr[i].p_filesz == 0)continue;

        size_t target_start=phdr[i].p_vaddr;
        target_start= target_start / 4096 * 4096; //align start: close interval

        size_t target_end= phdr[i].p_vaddr + phdr[i].p_memsz;
        target_end= (target_end + 4096 - 1) / 4096 * 4096; // align: open interval

        char buf[4096];

        for(size_t p=target_start; p < target_end; p+=4096){
            // p is pointer to page start
            memset(buf,0,4096);
            /**
             * target page range: [ p , p+4096 )
             * source page range: [ p-phdr[i].p_vaddr+phdr[i].p_offset , p-phdr[i].p_vaddr+phdr[i].p_offset+4096 )
             * available total source range: [ phdr[i].p_offset , phdr[i].p_offset+phdr[i].p_filesz )
             */

            size_t copy_start=max(
                    p-phdr[i].p_vaddr+phdr[i].p_offset,
                    phdr[i].p_offset
                    );
            size_t copy_end= min(
                    p-phdr[i].p_vaddr+phdr[i].p_offset+4096,
                    phdr[i].p_offset+phdr[i].p_filesz
                    );
            if(copy_start<copy_end){
                f_lseek(elf_file,copy_start);
                f_read(
                        elf_file,
                       buf+copy_start-(p-phdr[i].p_vaddr+phdr[i].p_offset),
                       copy_end-copy_start,&read_bytes
                       );
            }

            if(phdr[i].p_flags&PF_W){
                elf_control->bind_data_page(0x100000000+p, size_t(buf));
            }else{
                elf_control->bind_text_page(0x100000000+p, size_t(buf));
            }
        }
    }

//    PageTableUtil::FlushCurrentPageTable();
//
//    long long res=0;
//    for(char* p= (char*)(0x100000000+0x10000); p < (char*)(0x100000000+0x10000 + 0x19601e); p++){
//        res=(res*10007+*p)%1000000007;
//    }
//    printf("hash: 0x%x\n",res);
//
//    res=0;
//    for(char* p= (char*)(0x100000000+0x1a70a0); p < (char*)(0x100000000+0x1a70a0 + 0xbad8); p++){
//        res=(res*10007+*p)%1000000007;
//    }
//    printf("hash: 0x%x\n",res);

    *entry=Ehdr->e_entry;

    delete Ehdr;
#ifdef DEBUG_ELF
    printf("[ELF LOADER] ELF loaded successfully\n");
#endif
}
