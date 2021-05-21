#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "page.h"


#ifndef memory
#define __memory (8 * 1024 * 1024)
#else
#define __memory memory
#endif

#ifndef heap_page_num
#define __heap_page_num (8)
#else
#define __heap_page_num heap_page_num
#endif

#define __page_size (4096)
// #define __kernel_start (0x80200000)
#define __kernel_start (0x80020000)

void *k_malloc(size_t size);

void k_free(void *addr);

size_t get_page_num(void *page_addr);

/*
    Alloc Heap
 */
void memory_init();

#endif