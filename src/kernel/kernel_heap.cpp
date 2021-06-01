#include "memory/kernel_heap.h"

size_t __kernel_heap_base;

__Kernel_SegmentTreeNode kernel_heap_nodes[kernel_heap_size * 4 / __align_unit];
int __kernel_alloc_length[kernel_heap_size / __align_unit];

#define cnt kernel_heap_nodes[node]

int __kernel_get_size(int node) {
    return kernel_heap_nodes[node].r - kernel_heap_nodes[node].l + 1;
}

bool __kernel_isEmpty(int node) {
    return kernel_heap_nodes[node].max_space == __kernel_get_size(node);
}

void __kernel_push_up(int node) {
    assert(cnt.l != cnt.r);
    if (!__kernel_isEmpty(L(node))) {
        cnt.left_boarder_space = kernel_heap_nodes[L(node)].left_boarder_space;
    } else {
        cnt.left_boarder_space = __kernel_get_size(L(node)) + kernel_heap_nodes[R(node)].left_boarder_space;
    }
    cnt.left_right_space = kernel_heap_nodes[L(node)].right_boarder_space;
    cnt.right_left_space = kernel_heap_nodes[R(node)].left_boarder_space;
    if (!__kernel_isEmpty(R(node))) {
        cnt.right_boarder_space = kernel_heap_nodes[R(node)].right_boarder_space;
    } else {
        cnt.right_boarder_space = kernel_heap_nodes[L(node)].right_boarder_space + __kernel_get_size(R(node));
    }

    cnt.left_max_space = kernel_heap_nodes[L(node)].max_space;
    cnt.right_max_space = kernel_heap_nodes[R(node)].max_space;

    cnt.max_space = max(cnt.left_max_space, max(cnt.right_max_space, cnt.left_right_space + cnt.right_left_space));
}

void __kernel_init_pages(int node, int l, int r) {
    assert(kernel_heap_size * 4 / __align_unit);
    cnt.set = 0;
    cnt.l = l;
    cnt.r = r;
    cnt.mid = (l + r) / 2;

    cnt.max_space = r - l + 1;
    if (l != r) {
        __kernel_init_pages(L(node), l, cnt.mid);
        __kernel_init_pages(R(node), cnt.mid + 1, r);
        cnt.right_max_space = cnt.right_boarder_space = cnt.right_left_space = kernel_heap_nodes[R(node)].max_space;
        cnt.left_max_space = cnt.left_right_space = cnt.left_boarder_space = kernel_heap_nodes[L(node)].max_space;
    } else {
        cnt.right_max_space = cnt.right_boarder_space = cnt.right_left_space = 1;
        cnt.left_max_space = cnt.left_right_space = cnt.left_boarder_space = 1;
    }

}

int __kernel_find_space(int node, int size) {
    if (cnt.l == cnt.r) {
        // leaf node
        if (cnt.max_space >= size) {
            return cnt.l;
        } else {
            return -1;
        }
    } else {
        // try left son
        if (cnt.left_max_space >= size) {
            return __kernel_find_space(L(node), size);
        }
        // try right son
        if (cnt.right_max_space >= size) {
            return __kernel_find_space(R(node), size);
        }
        // try to place middle
        if (cnt.left_right_space + cnt.right_left_space >= size) {
            return cnt.mid - cnt.left_right_space + 1;
        }
        return -1;
    }
}

void __kernel_push_down(int node) {
    assert(cnt.l != cnt.r);
    if (cnt.set == 1) {
        kernel_heap_nodes[L(node)].set = cnt.set;
        kernel_heap_nodes[L(node)].max_space = kernel_heap_nodes[L(node)].left_boarder_space = kernel_heap_nodes[L(
                node)].right_boarder_space = 0;
        kernel_heap_nodes[R(node)].set = cnt.set;
        kernel_heap_nodes[R(node)].max_space = kernel_heap_nodes[R(node)].left_boarder_space = kernel_heap_nodes[R(
                node)].right_boarder_space = 0;
    } else if (cnt.set == -1) {
        kernel_heap_nodes[L(node)].set = cnt.set;
        kernel_heap_nodes[L(node)].max_space = kernel_heap_nodes[L(node)].left_boarder_space = kernel_heap_nodes[L(
                node)].right_boarder_space =
                kernel_heap_nodes[L(node)].r - kernel_heap_nodes[L(node)].l + 1;
        kernel_heap_nodes[R(node)].set = cnt.set;
        kernel_heap_nodes[R(node)].max_space = kernel_heap_nodes[R(node)].left_boarder_space = kernel_heap_nodes[R(
                node)].right_boarder_space =
                kernel_heap_nodes[R(node)].r - kernel_heap_nodes[R(node)].l + 1;
    }
    cnt.set = 0;
}

void __kernel_update(int node, int l, int r, int set) {
    l = max(l, cnt.l);
    r = min(r, cnt.r);
    if (r < l)return; // assert: l<=r

    if (l == cnt.l && r == cnt.r) {
        // full cover
        if (set == 1) {
            cnt.set = 1;
            cnt.max_space = cnt.left_boarder_space = cnt.right_boarder_space = 0;
        } else if (set == -1) {
            cnt.set = -1;
            cnt.max_space = cnt.left_boarder_space = cnt.right_boarder_space = cnt.r - cnt.l + 1;
        }
    } else {
        __kernel_push_down(node);
        __kernel_update(L(node), l, r, set);
        __kernel_update(R(node), l, r, set);
        __kernel_push_up(node);
    }
}

size_t total_alloc = 0;

void init_kernel_heap() {
    total_alloc = 0;
    __kernel_heap_base = alloc_page(kernel_heap_size);
    memset(kernel_heap_nodes, 0,
           sizeof(__Kernel_SegmentTreeNode) *
           kernel_heap_size * 4 / __align_unit);
    int page_count = kernel_heap_size / __align_unit;
    __kernel_init_pages(1, 0, page_count - 1);
}

void *k_malloc(size_t size) {
    int count = (size + __align_unit - 1) / __align_unit;
    total_alloc += count * __align_unit;
//    printf("alloc %d mem, tot=%d\n",count*__align_unit,total_alloc);
    int start = __kernel_find_space(1, count);
    if (start == -1) {
        printf("Can't alloc heap!\n");
        return (void *) -1;
    }
    __kernel_update(1, start, start + count - 1, 1);
    __kernel_alloc_length[start] = count;
//    printf("[alloc] size=%d %d - %d\n",count,start,start+count-1);
    return (void *) (__kernel_heap_base + start * __align_unit);
}

void k_free(size_t p) {
    int start = (p - __kernel_heap_base) / __align_unit;
    int length = __kernel_alloc_length[start];
    __kernel_update(1, start, start + length - 1, -1);
}

void *operator new(size_t size) {
    return k_malloc(size);
}

void *operator new[](size_t size) {
    return k_malloc(size);
}

void operator delete(void *p) {
    k_free((size_t) p);
}

void operator delete[](void *p) {
    k_free((size_t) p);
}

#undef cnt
