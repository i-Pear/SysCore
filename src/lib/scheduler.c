#include "scheduler.h"

int global_pid=20;

int get_new_pid(){
    return ++global_pid;
}

bool pcb_list_is_empty(pcb_List* list){
    return list->start==null;
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
    runnable.start=runnable.end=null;
    blocked.start=blocked.end=null;
    running=null;
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
    // read file
    int find = 0;
    struct Fat32Entry fat32Entry = fat_find_file_entry(elf_path, &find);
    if(!find){
        panic("can't find file");
    }
    int file_size = fat_calculate_file_size(fat32Entry);
    char *elf_file_cache = alloc_page(file_size);
    printf("Start read file...\n");
    fat_read_file(fat32Entry, elf_file_cache);
    printf("File read successfully.\n");

    size_t elf_page_base,entry;
    load_elf(elf_file_cache, fat_calculate_file_size(fat32Entry),&elf_page_base,&entry);
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
    pcb* new_pcb=k_malloc(sizeof(pcb));
    new_pcb->pid=get_new_pid();
    new_pcb->ppid=0;
    new_pcb->stack=stack_page;
    new_pcb->thread_context=thread_context;
    new_pcb->elf_page_base=elf_page_base;
    new_pcb->page_table=page_table_base;

    pcb_push_back(&runnable, new_pcb);
}

void yield(){
    pcb_push_back(&runnable, running);
    running=null;
    schedule();
}

void exit_process(){
    dealloc_page(running->elf_page_base);
    // TODO: dealloc_page(running->page_table);
    dealloc_page(running->stack);
    k_free(running->thread_context);
    k_free(running);
    running=null;
    schedule();
}

void schedule(){
    if(running!=null){
        __restore(running->thread_context);
    }else{
        if(pcb_list_is_empty(&runnable)){
            printf("Nothing to run, halt.\n");
//            shutdown();
            while (1);
        }else{
            running=runnable.start->pcb;
            lty(running);
            lty(running->elf_page_base);
            pcb_list_pop_front(&runnable);

            lty(running->thread_context->satp);
            lty(running->thread_context->sepc);

            printf("trying to restore\n");
            size_t* kernel_restore_context=__memory_end-sizeof(Context);
            memcpy(kernel_restore_context,running->thread_context, sizeof(Context));
            __restore();
        }
    }
}