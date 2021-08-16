#include "memory.h"
#include "Heap.h"
#include "../interrupt.h"

size_t __kernel_end;
int page_count;

size_t memory_pool[2048];
int memory_pointer; // point to the first available page

void init_memory(){
    // get kernel end
#ifdef QEMU
    __kernel_end=0x84000000;
#else
    __kernel_end=0x80100000;
#endif
    __kernel_end=(__kernel_end+__page_size-1)/__page_size*__page_size;
//    printf("kernel end= 0x%x\n",__kernel_end);

#ifdef QEMU
    page_count=1500;
#else
    page_count=(__memory_end-__kernel_end-sizeof(Context)-sizeof(size_t))/__page_size;
#endif
//    printf("[Memory] total global_pages' count is %d\n",page_count);

    for(int i=0; i<page_count; i++){
        memory_pool[i]=__kernel_end+__page_size*i;
    }
    memory_pointer=page_count-1;
}

size_t alloc_page(){
    if(memory_pointer<0)panic("Can't alloc page!")
    return memory_pool[memory_pointer--];
}

void dealloc_page(size_t p){
    if(p%0x1000)panic("Page not aligned!")
    memory_pool[++memory_pointer]=p;
}

bool is_page_alloced(size_t p){
    if(p%0x1000)panic("Page not aligned!")
    return p>=__kernel_end && p<=__kernel_end+__page_size*(page_count-1);
}

#undef cnt