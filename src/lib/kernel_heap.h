#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "stl.h"
#include "memory.h"

#define L(x) (x*2)
#define R(x) (x*2+1)

#define kernel_heap_size 4096*29
#define __align_unit 16

typedef struct {
    // left  node : [l,mid]
    // right node : [mid+1,r]
    int l, r;
    int mid;

    int left_boarder_space;
    int left_right_space;

    int right_left_space;
    int right_boarder_space;

    int left_max_space;
    int right_max_space;

    int max_space;

    // 0:not set; 1:set to occupied; -1:set to unoccupied
    int set;

} __Kernel_SegmentTreeNode;

extern __Kernel_SegmentTreeNode kernel_heap_nodes[];
extern int __kernel_alloc_length[];

int __kernel_get_size(int node);

bool __kernel_isEmpty(int node);

void __kernel_push_up(int node);

void __kernel_init_pages(int node, int l, int r);

int __kernel_find_space(int node, int size);

void __kernel_push_down(int node);

void __kernel_update(int node, int l, int r, int set);

void init_kernel_heap();

size_t k_malloc(size_t size);

void k_free(size_t p);

#endif