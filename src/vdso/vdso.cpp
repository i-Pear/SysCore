#include "vdso.h"
#include "../kernel/elf_loader.h"
#include "../kernel/memory/memory.h"
#include "vdso_text.h"
#include "vdso_data.h"

size_t vdso_text_page;
size_t vdso_data_page;

void init_vdso(){
    // copy vdso to a stand-along page
    vdso_text_page=alloc_page();
    memcpy((void*)(vdso_text_page), vdso_text_so, vdso_text_so_len);
    vdso_data_page=alloc_page();
    memcpy((void*)(vdso_data_page), vdso_data_so, vdso_data_so_len);

    auto* Ehdr=reinterpret_cast<Elf64_Ehdr*>(vdso_text_page);
    if (Ehdr->e_ident[0] != 0x7f || Ehdr->e_ident[1] != 0x45 || Ehdr->e_ident[2] != 0x4c || Ehdr->e_ident[3] != 0x46) {
        printf("[VDSO LOADER] Invalid ELF File! \n");
        shutdown();
    }
}

