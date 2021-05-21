#include "lib/page_table.h"
#include "lib/interrupt.h"
#include "lib/register.h"
#include "lib/elf_loader.h"
#include "driver/interface.h"
#include "driver/sdcard.h"
#include "lib/fat32.h"

void test_sdcard_main(){
    int find = 0;
    struct Fat32Entry fat32Entry = fat_find_file_entry("/lty", &find);
    char* buf = alloc_page(20*4096);
    int len = fat_read_file(fat32Entry, buf);
    printf("[FAT] file size = %d\n", fat32Entry.file_size);
    printf("[FAT] read %d Bytes\n", len);
}


/**
 * 此处打算通过 sret 进入u-mode
 * 中断后硬件会执行以下动作：
 * 异常指令的pc被保存在sepc中;pc被设置为stvec;
 * scause被设置为异常原因;stval被设置为出错地址或其他信息;
 * sie置零以禁止中断;之前的sie被保存到spie;
 * 之前的权限模式被保存到spp中;
 *
 * 之后的流程是我们手动完成的：保存现场;跳到中断处理函数;恢复现场;
 *
 * 所以这里需要恢复现场+将模拟硬件自动完成的动作。
 */
void init_thread() {
    printf("[OS] bsp init.\n");
    bsp_init();
    printf("[OS] sdcard init.\n");
    sdcard_init();
    printf("[OS] fat32 init.\n");
    fat32_init();
    // tree_all();
    printf("[OS] Starting to load elf...");
    int find = 0;
    struct Fat32Entry fat32Entry = fat_find_file_entry("/write", &find);
    int file_size = fat_calculate_file_size(fat32Entry);
    char* buf = alloc_page(20*4096);
    fat_read_file(fat32Entry, buf);
    // load ELF
    size_t ptr=load_elf(buf, file_size);
    printf("[OS] Prepare For User Mode.\n");
    printf("Testing write :\n");
    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    /**
     * 用户栈
     * 栈通常向低地址方向增长，故此处增加__page_size
     */
    thread_context.sp = (size_t) alloc_page(4096) + __page_size;
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
    // thread_context.sepc = (size_t) daemon_thread;
    thread_context.sepc = ptr;
    /**
     * 页表处理
     * 1. satp应由物理页首地址右移12位并且或上（8 << 60），表示开启sv39分页模式
     * 2. 未使用的页表项应该置0
     */
    size_t user_satp = (size_t) alloc_page(4096);

    for (int i = 0; i < 511; i++) {
        *((size_t *)user_satp + i) = 0;
    }

    // 0x8000_0000 -> 0x8000_0000
    *((size_t *)user_satp + 2) = (0x80000 << 10) | 0xdf;
    user_satp >>= 12;
    user_satp |= (8 << 60);
    thread_context.satp = user_satp;
    __restore(&thread_context);
}

/**
 * 这个函数用来使pc指向虚拟地址而不是真实地址
 * 切换之后仍然停留在s-mode
 */
void turn_to_virtual_supervisor_mode(){
    printf("[OS] Prepare For SuperVisor Mode With VirtualOffset.\n");
    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    thread_context.sp = register_read_sp();
    /**
     * 此处spp为1,表示s-mode
     */
    thread_context.sstatus |= REGISTER_SSTATUS_SPP; // spp = 1
    /**
     * 此处spie为1,表示s-mode允许中断
     */
    thread_context.sstatus |= REGISTER_SSTATUS_SPIE; // spie = 1
    /**
     * 此处sepc为中断后返回地址
     */
    thread_context.sepc = (size_t) init_thread;
    thread_context.ra = (size_t) init_thread;
    thread_context.satp = kernelContext.kernel_satp;
    __restore(&thread_context);
}

int main() {
    printf("[OS] Memory Init.\n");
    init_memory();
    init_kernel_heap();
    puts("[OS] Interrupt & Timer Interrupt Open.");
    kernelContext.kernel_satp = register_read_satp();
    interrupt_timer_init();

    turn_to_virtual_supervisor_mode();
    // unreachable
    puts("Press Any Key To Continue.");
    getchar();
    return 0;
}
