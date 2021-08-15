#ifndef OS_RISC_V_MEMORY_H
#define OS_RISC_V_MEMORY_H

#include "../stdbool.h"
#include "../stddef.h"
#include "../stdio.h"
#include "../../lib/stl/stl.h"

#define __page_size (4096)

#ifndef QEMU
#define __kernel_start (0x80020000)
#else
#define __kernel_start (0x80200000)
#endif

#define __memory_end  0x80000000+6*1024*1024
#define __kernel_stack_size 4096

extern size_t __kernel_end;

void init_memory();

size_t alloc_page();

void dealloc_page(size_t p);

bool is_page_alloced(size_t p);

#endif //OS_RISC_V_MEMORY_H
