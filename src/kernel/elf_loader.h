#ifndef OS_RISC_V_ELF_LOADER_H
#define OS_RISC_V_ELF_LOADER_H

#include <elf.h>
#include "stdio.h"
//#include "memory/kernel_heap.h"

//#define DEBUG_ELF

void load_elf(const char* elf_data,int size,size_t* elf_page_base,size_t* elf_page_size,size_t* entry,Elf64_Off* e_phoff,int* phnum,Elf64_Phdr ** kernel_phdr);

#endif //OS_RISC_V_ELF_LOADER_H
