#include "scheduler.h"
#include "../driver/fatfs/ff.h"
#include "self_test.h"
#include "vfs.h"
#include "kernel_stack.h"

int global_pid=1;

// Warning: when do sth with running, sync with latest running_context first
Context* running_context;

int get_new_pid(){
    return ++global_pid;
}

bool pcb_list_is_empty(pcb_List* list){
    return list->start==null;
}

char* get_running_cwd(){
    return running->cwd;
}

void file_describer_bind(size_t file_id,size_t real_file_describer){
    size_t_map_put(&running->occupied_file_describer,file_id,real_file_describer);
}

void file_describer_erase(size_t file_id){
    size_t_map_erase(&running->occupied_file_describer,file_id);
}

bool file_describer_exists(size_t file_id){
    return size_t_map_exists(&running->occupied_file_describer,file_id);
}

size_t file_describer_convert(size_t file_id){
    return size_t_map_get(&running->occupied_file_describer,file_id);
}

void bind_kernel_heap(size_t addr){
    size_t_list_push_back(&running->occupied_kernel_heap, addr);
}

void bind_pages(size_t addr){
    size_t_list_push_back(&running->occupied_pages, addr);
}

void pcb_push_back(pcb_List* list,pcb* pcb){
    if(list->start==null&&list->end==null){
        // empty list
        pcb_listNode* new_node=k_malloc(sizeof(pcb_listNode));
        lty(new_node);
        new_node->pcb=pcb;
        new_node->previous=null;
        new_node->next=null;

        list->start=list->end=new_node;
    }else{
        pcb_listNode* new_node=k_malloc(sizeof(pcb_listNode));
        new_node->pcb=pcb;
        // link
        new_node->previous=list->end;
        list->end->next=new_node;
        list->end=new_node;
    }
}

void pcb_push_front(pcb_List* list,pcb* pcb){
    if(list->start==null&&list->end==null){
        // empty list
        pcb_listNode* new_node=k_malloc(sizeof(pcb_listNode));
        new_node->pcb=pcb;
        new_node->previous=null;
        new_node->next=null;

        list->start=list->end=new_node;
    }else{
        pcb_listNode* new_node=k_malloc(sizeof(pcb_listNode));
        new_node->pcb=pcb;
        // link
        new_node->next=list->start;
        list->start->previous=new_node;
        list->start=new_node;
    }
}

void pcb_list_pop_front(pcb_List* list){
    if(list->start==list->end){
        // has only one
        pcb_listNode* firstNode=list->start;
        list->start=list->end=null;
        k_free(firstNode);
    }else{
        pcb_listNode* firstNode=list->start;
        firstNode->next->previous=null;
        list->start=firstNode->next;
        k_free(firstNode);
    }
}

pcb_List runnable,blocked;
pcb* running;

void init_scheduler(){
    running_context=0x80000000+6*1024*1024-sizeof(Context);
    runnable.start=runnable.end=null;
    blocked.start=blocked.end=null;
    running=null;
}

void clone(int flags,size_t stack,int ptid){
    if(flags!=17){
        printf("flag=%d\n",flags);
        panic("clone flags is not SIGCHLD, unknown todo.\n");
    }

    // sync with running_context
    *running->thread_context=*running_context;

    // copy context
    Context * child_context=new(Context);
    *child_context=*running->thread_context;

    // copy pcb
    pcb* child_pcb=new(pcb);
    *child_pcb=*running;
    child_pcb->thread_context=child_context;

    if(ptid!=0){
        child_pcb->ppid=ptid;
    }else{
        child_pcb->ppid=running->pid;
    }
    child_pcb->pid=get_new_pid();

    // set syscall return value
    running->thread_context->a0=child_pcb->pid;
    child_context->a0=0;
    
    // reset resource lists
    size_t_map_init(&child_pcb->occupied_file_describer);
    child_pcb->occupied_kernel_heap.start=child_pcb->occupied_kernel_heap.end=null;
    child_pcb->occupied_pages.start=child_pcb->occupied_pages.end=null;
    child_pcb->signal_list.start=child_pcb->signal_list.end=null;

    // copy file describer
    size_t_map_copy(&running->occupied_file_describer,&child_pcb->occupied_file_describer);
    // alloc file describer
    {
        size_t_mapNode * cnt=child_pcb->occupied_file_describer.start;
        while (cnt!=null){
            // increase file describer counter
            // TODO: increase file describer counter
            cnt=cnt->next;
        }
    }

    if(stack!=0){
        // fixed stack, will not copy spaces
        child_pcb->thread_context->sp=stack;
        child_pcb->thread_context->a0=0;
        child_context->sepc+=4;
        pcb_push_back(&runnable,child_pcb);
    }else{
        // fork, generate a new stack
        size_t stack= alloc_page(running->stack_size);
        memcpy(stack,running->stack,running->stack_size);

        size_t elf_page_base= alloc_page(running->elf_page_size);
        memcpy(elf_page_base,running->elf_page_base,running->elf_page_size);

        size_t page_table= alloc_page(4096);
        memset(page_table,0, 4096);
        *((size_t *) page_table + 2) = (0x80000 << 10) | 0xdf;
        child_context->satp = (page_table>>12)|(8LL << 60);
        child_context->sepc+=4;

        child_pcb->thread_context->a0=0;
        child_pcb->stack=stack;
        child_pcb->thread_context->sp=running->thread_context->sp-running->stack+stack;
        child_pcb->elf_page_base=elf_page_base;
        child_pcb->page_table=page_table;

        pcb_push_back(&runnable,child_pcb);
    }
    // sync with running_context
    *running_context=*running->thread_context;
}

size_t get_running_elf_page(){
    if(running==null)return 0;
    return running->elf_page_base;
}

int get_running_pid(){
    return running->pid;
}

int get_running_ppid(){
    return running->ppid;
}

void create_process(const char *elf_path) {
    Inode * inode = vfs_open((char * )elf_path, O_RDONLY, S_IFREG);
    if(inode == null){
        printf("open %s fail\n", elf_path);
        panic("")
    }
    int file_size = (int)inode->data->fat32.obj.objsize;
    char *elf_file_cache = alloc_page(file_size);
    printf("Start read file %s\n", elf_path);
    int read_bytes = vfs_read(inode, elf_file_cache, file_size);
    if(read_bytes < 0){
        printf("read %s fail\n", elf_path);
        panic("")
    }
    vfs_close(inode);
    printf("File read successfully.\n");

    size_t elf_page_base,entry,elf_page_size;
    load_elf(elf_file_cache, file_size,&elf_page_base,&elf_page_size,&entry);
    dealloc_page(elf_file_cache);

    Context* thread_context=new(Context);
    thread_context->sstatus = register_read_sstatus();
    /**
     * 用户栈
     * 栈通常向低地址方向增长，故此处增加__page_size
     */
     size_t stack_page=(size_t) alloc_page(4096);
     thread_context->sp = stack_page + __page_size;
//    size_t stack=(size_t) alloc_page(4096);
//    thread_context.sp=4096+0x40000000;
    /**
     * 此处spp应为0,表示user-mode
     */
    thread_context->sstatus |= REGISTER_SSTATUS_SPP; // spp = 1
    thread_context->sstatus ^= REGISTER_SSTATUS_SPP; // spp = 0
    /**
     * 此处spie应为1,表示user-mode允许中断
     */
    thread_context->sstatus |= REGISTER_SSTATUS_SPIE; // spie = 1
    /**
     * 此处sepc为中断后返回地址
     */
    thread_context->sepc = entry;
    /**
     * 页表处理
     * 1. satp应由物理页首地址右移12位并且或上（8 << 60），表示开启sv39分页模式
     * 2. 未使用的页表项应该置0
     */
    size_t page_table_base = (size_t) alloc_page(4096);
    memset(page_table_base,0,4096);
    // 0x8000_0000 -> 0x8000_0000
    *((size_t *) page_table_base + 2) = (0x80000 << 10) | 0xdf;
    thread_context->satp = (page_table_base>>12)|(8LL << 60);

    // push into runnable list
    pcb* child_pcb=k_malloc(sizeof(pcb));
    child_pcb->pid=get_new_pid();
    child_pcb->ppid=1;
    child_pcb->stack=stack_page;
    child_pcb->thread_context=thread_context;
    child_pcb->elf_page_base=elf_page_base;
    child_pcb->page_table=page_table_base;
    // TODO: 初始化工作目录为/，这不合理
    memset(child_pcb->cwd, 0, sizeof(child_pcb->cwd));
    child_pcb->cwd[0] = '/';

    child_pcb->elf_page_size=elf_page_size;
    child_pcb->stack_size=4096;

    // init lists
    child_pcb->occupied_file_describer.start=null;
    child_pcb->occupied_kernel_heap.start=child_pcb->occupied_kernel_heap.end=null;
    child_pcb->occupied_pages.start=child_pcb->occupied_pages.end=null;
    child_pcb->signal_list.start=child_pcb->signal_list.end=null;

    pcb_push_back(&runnable, child_pcb);
}

void yield(){
    // sync with running_context
    *running->thread_context=*running_context;
    running->thread_context->sepc+=4;

    pcb_push_back(&runnable, running);
    running=null;
    schedule();
}

void exit_process(){
//     dealloc_page(running->elf_page_base);
    // TODO: dealloc_page(running->page_table);
    // dealloc_page(running->stack);
    
    // free file describer
    // TODO: free file describer
    
    // TODO: free lists
    
    k_free(running->thread_context);
    k_free(running);
    running=null;
    schedule();
}

void schedule(){
    if(running!=null){
        // sync with running_context
        *running_context=*running->thread_context;

        __restore();
    }else{
        if(pcb_list_is_empty(&runnable)){
            if(has_next_test()){
                create_process(get_next_test());
                schedule();
            }else{
                printf("Nothing to run, halt.\n");
                while (1);
            }
        }else{
            // pick one to run
            running=runnable.start->pcb;
            lty(running);
            lty(running->elf_page_base);
            pcb_list_pop_front(&runnable);

            lty(running->thread_context->satp);
            lty(running->thread_context->sepc);

            // printf("trying to restore\n");

            // sync with running_context
            *running_context=*running->thread_context;
            __restore();
        }
    }
}