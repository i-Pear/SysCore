#include "Heap.h"
#include "memory.h"

HeapAllocator heap_allocator;

char heap_space[KERNEL_HEAP_PAGE_NUM * 4096];

void init_heap(){
    auto start = reinterpret_cast<size_t>(heap_space);
    memset(heap_space,0, sizeof(heap_space));
    heap_allocator.SetStart(start);
    heap_allocator.SetEnd(start + KERNEL_HEAP_PAGE_NUM * 4096);
    heap_allocator.Init();
}

void* k_malloc(size_t size){
    return (void*)heap_allocator.Alloc(size);
}

void k_free(size_t p){
    if(p != 0){
        heap_allocator.DeAlloc(p);
    }
}

void *operator new (size_t size){
    return k_malloc(size);
}

void *operator new[](size_t size){
    return k_malloc(size);
}

void operator delete(void *p){
    k_free((size_t)p);
}

void operator delete[](void *p){
    k_free((size_t)p);
}