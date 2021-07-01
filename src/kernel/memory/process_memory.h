#ifndef OS_RISC_V_PROCESS_MEMORY_H
#define OS_RISC_V_PROCESS_MEMORY_H

#include "../lib/stl/string.h"
#include "../lib/stl/map.h"
#include "memory.h"
#include "page_pool.h"
#include "../elf_loader.h"

class Process_memory_controller{

    String elf_path;

    // page table base
    size_t* page_table;

    // stack
    size_t stack_base;
    size_t stack_size;

    // elf segment data
    ELF_Info elf_segments;

    // virtual addr -> real addr
    // include: heap(can edit) / elf(.data can edit) / file(using mmap)
    Map<size_t,Mmap_unit> map;

};

#endif //OS_RISC_V_PROCESS_MEMORY_H
