#ifndef OS_RISC_V_HEAP_H
#define OS_RISC_V_HEAP_H

#define KERNEL_HEAP_PAGE_NUM (100)
#define KERNEL_HEAP_BIT_MAP_SIZE (KERNEL_HEAP_PAGE_NUM * 512 / 64)

#include "../stddef.h"
#include "../stdio.h"
#include "../../lib/stl/stl.h"

class HeapAllocator {
public:

    void SetStart(size_t start){
        start_ = start;
    }

    void SetEnd(size_t end){
        end_ = end;
    }

    // there are some examples
    // init -> 0 0 0 0 ...
    // alloc 8 -> 0 1 0 0 ...
    // alloc 8 -> 0 1 0 1 0 ...
    // dealloc 8 -> 0 0 0 1 0 ...
    // alloc 16 -> 0 0 0 1 0 1 1 0 ...
    size_t Alloc(size_t size) {
        printf("[heap alloc] 0x%x\n", size);
        size_t bits = (size + 8) / 8;
        size_t i = 0;
        while (true){
            auto x = Get(i);
            if(x == 0){
                i++;
                Check(i);
                if(Get(i) != 0){
                    continue;
                }
                size_t flag = 1;
                for(size_t j = 0;j <= bits; j++){
                    if(Get(i + j) == 1){
                        flag = 0;
                        break;
                    }
                }
                if(flag == 0){
                    i += bits - 1;
                    Check(i);
                }else{
                    for(size_t j = 0;j < bits; j++){
                        Set(i + j, 1);
                    }
                    return start_ + i * 8;
                }
            }else{
                i++;
                Check(i);
            }
        }
    }

    void DeAlloc(size_t address) {
        size_t start = address - start_;
        if(start < 0 || start > end_){
            printf("[DeAlloc] address = 0x%x", address);
            panic("DeAlloc Error")
        }
        size_t bits = start / 8;
        while (Get(bits) == 1){
            Set(bits, 0);
            bits++;
        }
    }

    void Init() {
        for (size_t i = 0; i < KERNEL_HEAP_BIT_MAP_SIZE; i++) {
            bit_map_[i] = 0;
        }
    }
private:
    void Check(size_t i){
        if(i < 0 || i > KERNEL_HEAP_BIT_MAP_SIZE * 64){
            panic("kernel heap no space")
        }
    }

    size_t Get(size_t bit) {
        size_t index = (bit / 64);
        size_t offset = (63 - (bit % 64));
        return (bit_map_[index] & (1LL << offset)) ? 1 : 0;
    }

    void Set(size_t bit, size_t value) {
        size_t index = (bit / 64);
        size_t offset = (63 - (bit % 64));
        bit_map_[index] = bit_map_[index] | (1LL << offset);
        if (value == 0) {
            bit_map_[index] = bit_map_[index] ^ (1LL << offset);
        }
    }

    size_t start_;
    size_t end_;

    size_t bit_map_[KERNEL_HEAP_BIT_MAP_SIZE];
};

extern HeapAllocator heap_allocator;
void init_heap();

void* k_malloc(size_t size);

void k_free(size_t p);

void *operator new (size_t size);

void *operator new[](size_t size);

void operator delete(void *p);

void operator delete[](void *p);

#endif //OS_RISC_V_HEAP_H
