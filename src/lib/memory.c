#include "memory.h"

#define L(x) ((x) << 1)
#define R(x) (((x) << 1) + 1)

#define __page_num (__memory / __page_size)
#define __heap_size (__heap_page_num * __page_size)

typedef struct {
    bool is_alloced;
    bool is_split;
} Buddy;

Buddy buddies[(__heap_size) << 2] = {};
size_t kernel_end = 0;

size_t fix_size(size_t x) {
    x -= 1;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

bool alloc(size_t size, size_t cur, size_t *addr, size_t csize) {
    if (buddies[cur].is_alloced || (cur >= __heap_size << 2) || csize < size) {
        return false;
    }
    if (csize == size && !buddies[cur].is_split) {
        buddies[cur].is_alloced = true;
        return true;
    }
    if (!buddies[L(cur)].is_alloced) {
        bool ret = alloc(size, L(cur), addr, csize >> 1);
        if (ret) {
            buddies[cur].is_split = true;
            return ret;
        }
    }
    if (!buddies[R(cur)].is_alloced) {
        *addr += csize >> 1;
        bool ret = alloc(size, R(cur), addr, csize >> 1);
        if (ret) {
            buddies[cur].is_split = true;
            return ret;
        }
    }
    return false;
}

void *k_malloc(size_t size) {
    size = fix_size(size);
    size_t addr = kernel_end;
    bool success = alloc(size, 1, &addr, __heap_size);
    if (!success) {
        puts("Memory overflow.\n");
        return 0;
    }
    return (void *) addr;
}

void dealloc(size_t addr, size_t cur, size_t ptr, size_t csize) {
    size_t m = csize >> 1;
    if (cur >= __heap_size << 2) {
        return;
    }
    if (buddies[cur].is_alloced && addr == ptr) {
        buddies[cur].is_alloced = false;
    } else if (addr < m && buddies[cur].is_split) {
        dealloc(addr, L(cur), ptr, csize >> 1);
    } else if (addr > m && buddies[cur].is_split) {
        dealloc(addr, R(cur), ptr + (csize >> 1), csize >> 1);
    }
    if (!buddies[cur].is_alloced && buddies[cur].is_split &&
        !buddies[L(cur)].is_split && !buddies[L(cur)].is_alloced &&
        !buddies[R(cur)].is_split && !buddies[R(cur)].is_alloced)
        buddies[cur].is_split = false;
}

void k_free(void *addr) {
    dealloc((size_t) addr - get_kernel_end(), 1, 0, __heap_size);
}

void memory_init() {
    printf("memory_init\n");
    size_t used_page_num = __heap_page_num + 10;
    kernel_end = alloc_page(used_page_num * 4096);
}
