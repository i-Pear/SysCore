#include "lib/page_table.h"
#include "lib/interrupt.h"
#include "lib/register.h"
#include "lib/elf_loader.h"
#include "driver/interface.h"
#include "driver/sdcard.h"
#include "lib/fat32.h"
#include "lib/scheduler.h"

void test_sdcard_main(){
    int find = 0;
    struct Fat32Entry fat32Entry = fat_find_file_entry("/lty", &find);
    char* buf = alloc_page(20*4096);
    int len = fat_read_file(fat32Entry, buf);
    printf("[FAT] file size = %d\n", fat32Entry.file_size);
    printf("[FAT] read %d Bytes\n", len);
}

void print_satp(){
    lty(register_read_satp());
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
    printf("[OS] Interrupt & Timer Interrupt Open.\n");
    interrupt_timer_init();
    printf("[OS] init scheduler.\n");
    init_scheduler();

    create_process("/riscv64/write");
    create_process("/riscv64/uname");
//    create_process("/riscv64/times");
//    create_process("/riscv64/getpid");
//    create_process("/riscv64/getppid");

    schedule();
}

int main() {
    printf("[OS] Memory Init.\n");
    init_memory();
    init_kernel_heap();
//    puts("[OS] Interrupt & Timer Interrupt Open.");
    kernelContext.kernel_satp = register_read_satp() | (8LL << 60);
    lty(kernelContext.kernel_satp);
    kernelContext.kernel_handle_interrupt = (size_t)handle_interrupt;
    kernelContext.kernel_restore = (size_t) __restore;
//    interrupt_timer_init();

    init_thread();
    // unreachable
    panic("Unreachable code!");
    return 0;
}
