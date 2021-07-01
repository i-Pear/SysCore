#ifndef OS_RISC_V_ELF_LOADER_H
#define OS_RISC_V_ELF_LOADER_H

#include <elf.h>
#include "stdio.h"
#include "memory/kernel_heap.h"
#include "../lib/stl/list.h"

//#define DEBUG_ELF

void load_elf(const char* elf_data,int size,size_t* elf_page_base,size_t* elf_page_size,size_t* entry);

struct ELF_Segment{
    size_t file_offset;
    size_t memory_offset;
    size_t file_size;
    size_t memory_size;
};

struct ELF_Info{
    List<ELF_Segment> segments;
};

#endif //OS_RISC_V_ELF_LOADER_H
