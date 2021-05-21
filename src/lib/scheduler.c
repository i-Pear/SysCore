#include "scheduler.h"
#include "fat32.h"
#include "elf_loader.h"
#include "interrupt.h"
#include "register.h"

struct pcb {
    int pid;
    size_t stack;
    size_t executable;
};

void create_process(const char *elf_path) {
    // read file
    int find = 0;
    struct Fat32Entry fat32Entry = fat_find_file_entry(elf_path, &find);
    int file_size = fat_calculate_file_size(fat32Entry);
    char *elf_file_cache = alloc_page(file_size);
    printf("Start read file...\n");
    fat_read_file(fat32Entry, elf_file_cache);
    printf("File read successfully.\n");

    size_t elf_page_base,entry;
    load_elf(elf_file_cache, file_size,&elf_page_base,&entry);

    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    /**
     * 用户栈
     * 栈通常向低地址方向增长，故此处增加__page_size
     */
     thread_context.sp = (size_t) alloc_page(4096) + __page_size;
//    size_t stack=(size_t) alloc_page(4096);
//    thread_context.sp=4096+0x40000000;
    /**
     * 此处spp应为0,表示user-mode
     */
    thread_context.sstatus |= REGISTER_SSTATUS_SPP; // spp = 1
    thread_context.sstatus ^= REGISTER_SSTATUS_SPP; // spp = 0
    /**
     * 此处spie应为1,表示user-mode允许中断
     */
    thread_context.sstatus |= REGISTER_SSTATUS_SPIE; // spie = 1
    /**
     * 此处sepc为中断后返回地址
     */
    thread_context.sepc = entry;
    /**
     * 页表处理
     * 1. satp应由物理页首地址右移12位并且或上（8 << 60），表示开启sv39分页模式
     * 2. 未使用的页表项应该置0
     */
    size_t page_table_base = (size_t) alloc_page(4096);

    for (int i = 0; i < 512; i++) {
        *((size_t *) page_table_base + i) = 0;
    }

    // 0x8000_0000 -> 0x8000_0000
    *((size_t *) page_table_base + 2) = (0x80000 << 10) | 0xdf;

    page_table_base >>= 12;
    page_table_base |= (8LL << 60);
    lty(page_table_base);
    thread_context.satp = page_table_base;
    lty(register_read_satp());
    __restore(&thread_context);
}