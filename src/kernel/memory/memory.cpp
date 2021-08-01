#include "memory.h"
#include "Heap.h"

size_t __kernel_end;
int page_count;

__Memory_SegmentTreeNode global_pages[__reserved_page_num * 4];
int __memory_alloc_length[__reserved_page_num];

#define cnt global_pages[node]

int __memory_get_size(int node){
    return global_pages[node].r - global_pages[node].l + 1;
}

bool __memory_isEmpty(int node){
    return global_pages[node].max_space == __memory_get_size(node);
}

void __memory_push_up(int node) {
    if(!__memory_isEmpty(L(node))){
        cnt.left_boarder_space = global_pages[L(node)].left_boarder_space;
    }else{
        cnt.left_boarder_space = __memory_get_size(L(node)) + global_pages[R(node)].left_boarder_space;
    }
    cnt.left_right_space = global_pages[L(node)].right_boarder_space;
    cnt.right_left_space = global_pages[R(node)].left_boarder_space;
    if(!__memory_isEmpty(R(node))){
        cnt.right_boarder_space = global_pages[R(node)].right_boarder_space;
    }else{
        cnt.right_boarder_space = global_pages[L(node)].right_boarder_space + __memory_get_size(R(node));
    }

    cnt.left_max_space = global_pages[L(node)].max_space;
    cnt.right_max_space = global_pages[R(node)].max_space;

    cnt.max_space = max(cnt.left_max_space, max(cnt.right_max_space, cnt.left_right_space + cnt.right_left_space));
}

void __memory_init_pages(int node, int l, int r) {
    cnt.set = 0;
    cnt.l = l;
    cnt.r = r;
    cnt.mid = (l + r) / 2;

    cnt.max_space= r - l + 1;
    if(l!=r){
        __memory_init_pages(L(node),l,cnt.mid);
        __memory_init_pages(R(node),cnt.mid+1,r);
        cnt.right_max_space =cnt.right_boarder_space = cnt.right_left_space = global_pages[R(node)].max_space;
        cnt.left_max_space =cnt.left_right_space = cnt.left_boarder_space = global_pages[L(node)].max_space;
    }else{
        cnt.right_max_space =cnt.right_boarder_space = cnt.right_left_space = 1;
        cnt.left_max_space =cnt.left_right_space = cnt.left_boarder_space = 1;
    }

}

int __memory_find_space(int node, int size) {
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
            return __memory_find_space(L(node), size);
        }
        // try right son
        if (cnt.right_max_space >= size) {
            return __memory_find_space(R(node), size);
        }
        // try to place middle
        if (cnt.left_right_space + cnt.right_left_space >= size) {
            return cnt.mid - cnt.left_right_space + 1;
        }
        return -1;
    }
}

void __memory_push_down(int node) {
    if (cnt.set == 1) {
        global_pages[L(node)].set = cnt.set;
        global_pages[L(node)].max_space = global_pages[L(node)].left_boarder_space = global_pages[L(node)].right_boarder_space = 0;
        global_pages[R(node)].set = cnt.set;
        global_pages[R(node)].max_space = global_pages[R(node)].left_boarder_space = global_pages[R(node)].right_boarder_space = 0;
    } else if (cnt.set == -1) {
        global_pages[L(node)].set = cnt.set;
        global_pages[L(node)].max_space = global_pages[L(node)].left_boarder_space = global_pages[L(node)].right_boarder_space =
                global_pages[L(node)].r - global_pages[L(node)].l + 1;
        global_pages[R(node)].set = cnt.set;
        global_pages[R(node)].max_space = global_pages[R(node)].left_boarder_space = global_pages[R(node)].right_boarder_space =
                global_pages[R(node)].r - global_pages[R(node)].l + 1;
    }
    cnt.set = 0;
}

void __memory_update(int node, int l, int r, int set) {
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
        __memory_push_down(node);
        __memory_update(L(node), l, r, set);
        __memory_update(R(node), l, r, set);
        __memory_push_up(node);
    }
}

void init_memory(){
    memset(global_pages, 0, sizeof(global_pages));
    printf("Reserved page num=%d\n", __reserved_page_num);
    printf("origin kernel_end: 0x%x\n",get_kernel_end());
    __kernel_end=get_kernel_end();
    __kernel_end=(__kernel_end+__page_size-1)/__page_size*__page_size;
    printf("kernel end= 0x%x\n",__kernel_end);
    int page_count=(__memory_end-__kernel_end)/__page_size;
#ifdef QEMU
    page_count=1500;
#endif
    printf("[Memory] total global_pages' count is %d\n",page_count);
    __memory_init_pages(1,0,page_count-1);
}

size_t alloc_page(size_t size){
//     printf("Trying to alloc page, size=0x%x\n",size);
    int count=(size+__page_size-1)/__page_size;
//     printf("count=%d\n",count);
    int start=__memory_find_space(1,count);
    if(start==-1){
        printf("Can't alloc page!\n");
        shutdown();
        return -1;
    }
    // printf("Alloced page of start=0x%x end=0x%x\n",__kernel_end+start*__page_size,__kernel_end+start*__page_size+count*__page_size);
    __memory_update(1,start,start+count-1,1);
    __memory_alloc_length[start]=count;
    //memset((void*)(__kernel_end + start * __page_size), 0, __page_size * count);
    return __kernel_end+start*__page_size;
}

void dealloc_page(size_t p){
    int start=(p-__kernel_end)/__page_size;
    int length=__memory_alloc_length[start];
    // printf("dealloc_page page: start=%d length=%d\n",start,length);
    __memory_update(1,start,start+length-1,-1);
}

#undef cnt