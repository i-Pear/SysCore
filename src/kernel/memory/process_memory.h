#ifndef OS_RISC_V_PROCESS_MEMORY_H
#define OS_RISC_V_PROCESS_MEMORY_H

#include "../lib/stl/string.h"
#include "../lib/stl/map.h"
#include "memory.h"
#include "page_pool.h"

class Process_memory_controller{
    String elf_path;
    size_t* page_table;

    size_t stack_size;
    size_t elf_page_size;
    size_t stack;

    // virtual addr -> real addr
    Map<size_t,Mmap_unit> map;

};

#endif //OS_RISC_V_PROCESS_MEMORY_H
