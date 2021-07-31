#include "scheduler.h"
#include "self_test.h"
#include "memory/kernel_stack.h"
//#include "elf_data.h"
extern "C"{
#include "../driver/fatfs/ff.h"
}


int global_pid=1;

// Warning: when do sth with running, sync with latest running_context first
Context* running_context;

PCB* search_by_pid(int pid){
    // search in running
    if(running!=nullptr&&running->pid==pid)return running;
    // search in runnable
    auto cnt=runnable.start;
    while (cnt!=nullptr){
        // printf(">> runnable pid=%d\n",cnt->PCB->pid);
        if(cnt->data->pid==pid)return cnt->data;
        cnt=cnt->next;
    }
    // search in blocked
    cnt=blocked.start;
    while (cnt!=nullptr){
        // printf(">> blocked pid=%d\n",cnt->PCB->pid);
        if(cnt->data->pid==pid)return cnt->data;
        cnt=cnt->next;
    }
    return nullptr;
}

int get_new_pid(){
    return ++global_pid;
}

char* get_running_cwd(){
    return running->cwd;
}

void file_describer_bind(size_t file_id,size_t real_file_describer){
    running->occupied_file_describer.put(file_id,real_file_describer);
}

void file_describer_erase(size_t file_id){
    running->occupied_file_describer.erase(file_id);
}

bool file_describer_exists(size_t file_id){
    return running->occupied_file_describer.exists(file_id);
}

size_t file_describer_convert(size_t file_id){
    return running->occupied_file_describer.get(file_id);
}

void bind_kernel_heap(size_t addr){
    running->occupied_kernel_heap->push_back(addr);
}

void bind_pages(size_t addr){
    running->occupied_pages.push_back(addr);
}

List<PCB*> runnable,blocked;
PCB* running;

void init_scheduler(){
#ifndef QEMU
    running_context= reinterpret_cast<Context *>(0x80000000 + 6 * 1024 * 1024 - sizeof(Context));
#else
    running_context= reinterpret_cast<Context *>(0x89000000 + 6 * 1024 * 1024 - sizeof(Context));
#endif
    runnable.start=runnable.end=nullptr;
    blocked.start=blocked.end=nullptr;
    running=nullptr;
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

    // copy PCB
    PCB* child_pcb=new(PCB);
    *child_pcb=*running;
    child_pcb->thread_context=child_context;

    if(stack!=0&&ptid!=0){
        child_pcb->ppid=ptid;
    }else{
        child_pcb->ppid=running->pid;
    }
    child_pcb->pid=get_new_pid();

    // set syscall return value
    running->thread_context->a0=child_pcb->pid;
    child_context->a0=0;

    // copy file describer
    child_pcb->occupied_file_describer=running->occupied_file_describer;
    // alloc file describer
    {
        auto * cnt=child_pcb->occupied_file_describer.data.start;
        while (cnt!=nullptr){
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
        runnable.push_back(child_pcb);
    }else{
        // fork, generate a new stack
        size_t stack= alloc_page(running->stack_size);
        memcpy(reinterpret_cast<void *>(stack), reinterpret_cast<const void *>(running->stack), running->stack_size);

        size_t elf_page_base= alloc_page(running->elf_page_size);
        memcpy(reinterpret_cast<void *>(elf_page_base), reinterpret_cast<const void *>(running->elf_page_base), running->elf_page_size);

        size_t page_table= alloc_page(4096);
        memset(reinterpret_cast<void *>(page_table), 0, 4096);
        *((size_t *) page_table + 2) = (0x80000 << 10) | 0xdf;
        child_context->satp = (page_table>>12)|(8LL << 60);
        child_context->sepc+=4;

        child_pcb->thread_context->a0=0;
        child_pcb->stack=stack;
        child_pcb->thread_context->sp=running->thread_context->sp-running->stack+stack;
        child_pcb->elf_page_base=elf_page_base;
        child_pcb->page_table=page_table;

        runnable.push_back(child_pcb);
    }
    // sync with running_context
    *running_context=*running->thread_context;
}

size_t get_running_elf_page(){
    if(running==nullptr)return 0;
    return running->elf_page_base;
}

int get_running_pid(){
    return running->pid;
}

int get_running_ppid(){
    return running->ppid;
}

void create_process(const char *elf_path) {
    FIL fnew;
    printf("elf %s\n", elf_path);
    int res = f_open(&fnew, elf_path, FA_READ);
    if(res != FR_OK){
        panic("read error")
    }
    int file_size = fnew.obj.objsize;
    char *elf_file_cache = (char *)alloc_page(file_size);
    printf("Start read file...\n");
    uint32_t read_bytes;
    f_read(&fnew, elf_file_cache, file_size, &read_bytes);
    f_close(&fnew);
    printf("File read successfully.\n");
    size_t elf_page_base,entry,elf_page_size;
    load_elf(elf_file_cache, file_size, &elf_page_base, &elf_page_size, &entry);
    // dealloc_page(elf_file_cache);

    Context* thread_context=new(Context);
    thread_context->sstatus = register_read_sstatus();
    /**
     * 用户栈
     * 栈通常向低地址方向增长，故此处增加__page_size
     */
     size_t stack_page=(size_t) alloc_page(4096);
     memset(reinterpret_cast<void *>(stack_page), 0, 4096);
     thread_context->sp = stack_page + __page_size-2*8;
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
    memset(reinterpret_cast<void *>(page_table_base), 0, 4096);
    // 0x8000_0000 -> 0x8000_0000
    *((size_t *) page_table_base + 2) = (0x80000 << 10) | 0xdf;
    thread_context->satp = (page_table_base>>12)|(8LL << 60);

    // push into runnable list
    PCB* new_pcb= new PCB;
    new_pcb->pid=get_new_pid();
    new_pcb->ppid=1;
    new_pcb->stack=stack_page;
    new_pcb->thread_context=thread_context;
    new_pcb->elf_page_base=elf_page_base;
    new_pcb->page_table=page_table_base;
    // TODO: 初始化工作目录为/，这不合理
    memset(new_pcb->cwd, 0, sizeof(new_pcb->cwd));
    new_pcb->cwd[0] = '/';

    new_pcb->elf_page_size=elf_page_size;
    new_pcb->stack_size=4096;

    runnable.push_back(new_pcb);
}

void yield(){
    // sync with running_context
    *running->thread_context=*running_context;
    running->thread_context->sepc+=4;

    runnable.push_back(running);
    running=nullptr;
    schedule();
}

void execute(const char* exec_path){
    // deal with path "/"
    if(exec_path[0]!='/'){
        char buf[strlen(exec_path)+ strlen(running->cwd)+2];
        if(strcmp(running->cwd,"/")==0){
            sprintf(buf,"/%s",exec_path);
        }else{
            sprintf(buf,"%s/%s",running->cwd,exec_path);
        }
        create_process(exec_path);
        exit_process(0);
    }else{
        create_process(exec_path);
        exit_process(0);
    }
}

void exit_process(int exit_ret){
    // printf("process %d start to exit\n",running->pid);
    // send signal
    PCB* parent= search_by_pid(running->ppid);
    if(parent!=nullptr){
        // printf("unfreeze pid=%d\n",parent->pid);
        parent->signal_list.push_back(make_pair(running->pid,exit_ret));
    }
//     dealloc_page(running->elf_page_base);
    // TODO: dealloc_page(running->page_table);
    // dealloc_page(running->stack);

    // free file describer
    // TODO: free file describer

    // TODO: free lists

    delete running->thread_context;
    delete running;
    running=nullptr;
    schedule();
}

void schedule(){
    if(running!=nullptr){
        // sync with running_context
        *running_context=*running->thread_context;

        __restore();
    }else{
        if(!runnable.is_empty()){
            // pick one to run
            running=runnable.start->data;
            lty(running);
            lty(running->elf_page_base);
            runnable.pop_front();

            lty(running->thread_context->satp);
            lty(running->thread_context->sepc);

            // printf("trying to restore\n");

            // sync with running_context
            *running_context=*running->thread_context;

            __restore();
        }else if(!blocked.is_empty()){
            // search one to unfreeze
            auto cnt=blocked.start;
            while (cnt!=nullptr){
                if(!cnt->data->signal_list.is_empty()){
                    // has signal, wake up
                    running=cnt->data;
                    // remove it from blocked list
                    blocked.erase(cnt);
                    // get signal to return value
                    running->thread_context->a0=running->signal_list.start->data.first;
                    *running->wstatus=running->signal_list.start->data.second<<8;
                    running->signal_list.pop_front();

                    schedule();
                }
                cnt=cnt->next;
            }
            panic("There exists a dead waiting loop. All processes are blocked.");
        }else{
            if(has_next_test()){
                create_process(get_next_test());
                schedule();
            }
            else{
                printf("Nothing to run, halt.\n");
                while (1);
            }
        }
    }
}

int wait(int* wstatus){
//    printf("process %d start to wait\n",running->pid);
    if(!running->signal_list.is_empty()){
        // return immediately
        int ret=running->signal_list.start->data.first;
        *wstatus=running->signal_list.start->data.second<<8;
        running->signal_list.pop_front();
        return ret;
    }

    // move to blocked list

    // sync with running_context
    *running->thread_context=*running_context;
    running->thread_context->sepc+=4;
    running->wait_pid=0;
    running->wstatus=wstatus;

    blocked.push_back(running);
    running=nullptr;
    schedule();
}

PCB::PCB(const PCB &other):occupied_file_describer(other.occupied_file_describer){
    pid=other.pid;
    ppid=other.ppid;
    stack=other.stack;
    elf_page_base=other.elf_page_base;
    page_table=other.page_table;

    stack_size=other.stack_size;
    elf_page_size=other.elf_page_size;
    occupied_kernel_heap = other.occupied_kernel_heap;

    thread_context=nullptr;
    strcpy(cwd,other.cwd);

}

PCB::PCB(): occupied_kernel_heap(new List<size_t>){

}
