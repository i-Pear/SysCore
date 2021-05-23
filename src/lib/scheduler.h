#ifndef OS_RISC_V_SCHEDULER_H
#define OS_RISC_V_SCHEDULER_H

#include "stl.h"
#include "elf_loader.h"
#include "interrupt.h"
#include "register.h"

#define MAX_PATH_LENGTH 32

extern int global_pid;

typedef struct{
    int pid;
    int ppid;
    size_t stack;
    size_t elf_page_base;
    size_t page_table;

    size_t stack_size;
    size_t elf_page_size;

    Context * thread_context;
    char cwd[MAX_PATH_LENGTH];
} pcb;

typedef struct pcb_listNode{
    pcb* pcb;
    struct pcb_listNode* previous;
    struct pcb_listNode* next;
} pcb_listNode;

typedef struct {
    pcb_listNode* start;
    pcb_listNode* end;
} pcb_List;

extern pcb_List runnable,blocked;
extern pcb* running;

int get_new_pid();

char* get_running_cwd();

bool pcb_list_is_empty(pcb_List* list);

void pcb_push_back(pcb_List* list,pcb* pcb);

void pcb_push_front(pcb_List* list,pcb* pcb);

void pcb_list_pop_front(pcb_List* list);

void init_scheduler();

size_t get_running_elf_page();

int get_running_pid();

int get_running_ppid();

void create_process(const char *elf_path);

void clone(int flags,size_t stack,int ptid);

void yield();

void exit_process();

void schedule();

#endif //OS_RISC_V_SCHEDULER_H
