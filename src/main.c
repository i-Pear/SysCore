#include "lib/page_table.h"
#include "lib/interrupt.h"
#include "lib/register.h"
#include "driver/interface.h"
#include "lib/scheduler.h"
#include "lib/file_describer.h"


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
    printf("[OS] Init Fs.\n");
    FATFS fs;
    FRESULT res_sd;
    res_sd = f_mount(&fs, "", 1);
    if(res_sd != FR_OK){
        panic("fat init failed")
    }
    printf("[OS] Interrupt & Timer Interrupt Open.\n");
    interrupt_timer_init();
    printf("[OS] init scheduler.\n");
    init_scheduler();
    init_file_describer();

    create_process("write");
    create_process("uname");
    create_process("times");
    create_process("getpid");
    create_process("getppid");
    create_process("read");
    create_process("open");
//    create_process("openat");

    schedule();
}

int main() {
    printf("[OS] Memory Init.\n");
    init_memory();
    init_kernel_heap();
    kernelContext.kernel_satp = register_read_satp() | (8LL << 60);
    lty(kernelContext.kernel_satp);
    kernelContext.kernel_handle_interrupt = (size_t)handle_interrupt;
    kernelContext.kernel_restore = (size_t) __restore;

    init_thread();
    // unreachable
    panic("Unreachable code!");
    return 0;
}
