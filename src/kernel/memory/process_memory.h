#ifndef OS_RISC_V_PROCESS_MEMORY_H
#define OS_RISC_V_PROCESS_MEMORY_H

#include "../../lib/stl/string.h"
#include "../../lib/stl/map.h"
#include "memory.h"
#include "../../lib/stl/list.h"
#include "../elf_loader.h"

#define stack_base 0x20000000

struct Process_Memory{

    // page table base
    size_t* page_table;

    // elf file
    size_t elf_base;

    // heap
#define heap_base 0x40000000
    List<size_t> heap_maps; // index -> physical address

};

#endif //OS_RISC_V_PROCESS_MEMORY_H
