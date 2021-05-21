#ifndef OS_RISC_V_ELF_LOADER_H
#define OS_RISC_V_ELF_LOADER_H

#include <elf.h>
#include "stdio.h"
#include "kernel_heap.h"

void* elf_read(void** source, int size);

void load_elf(const char* elf_data,int size,size_t* elf_page_base,size_t* entry);

#endif //OS_RISC_V_ELF_LOADER_H
