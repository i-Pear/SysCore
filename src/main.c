#include "lib/page.h"
#include "lib/interrupt.h"
#include "lib/register.h"

void D(size_t x) { printf("0x%x\n", x); }

void daemon_thread() {
    asm volatile ("li a7, 0");
    asm volatile ("li a1, 999");
    asm volatile ("li a0, 11");
    asm volatile ("ecall");


    while (1);
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
    printf("[DEBUG] Prepare For User Mode.\n");
    kernelContext.kernel_satp = register_read_satp();
    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    // kernel stack
    thread_context.sp = register_read_sp();
    thread_context.sstatus |= REGISTER_SSTATUS_SPP; // spp = 1
    thread_context.sstatus ^= REGISTER_SSTATUS_SPP; // spp = 0
    thread_context.sstatus |= REGISTER_SSTATUS_SPIE; // spie = 1
    thread_context.sepc = (size_t) daemon_thread;
    thread_context.sepc += __kernel_vir_offset;
    thread_context.ra = (size_t) daemon_thread;
    thread_context.ra += __kernel_vir_offset;
    size_t user_satp = (size_t)alloc_page();
    size_t* virtual_user_satp = (size_t*)(user_satp + __kernel_vir_offset);
    for(int i = 0;i < 511; i++){
        *(virtual_user_satp + i) = 0;
    }
    *(virtual_user_satp + 2) = (0x80000 << 10) | 0xdf;
    *(virtual_user_satp + 510) = (0x80000 << 10) | 0xdf;
    user_satp >>= 12;
    user_satp |= (8 << 60);
    thread_context.satp = user_satp;
    __restore(&thread_context);
}

void print_sp(){
    printf("sp = 0x%x\n", register_read_sp());
}

void print_satp(){
    printf("satp = 0x%x\n", register_read_satp());
}

int main(size_t hart_id, size_t dtb_pa) {
    printf("[DEBUG] Memory Init.\n");
    memory_init();
    puts("[DEBUG] Timer Interrupt Start.");
    interrupt_timer_init();

    init_thread();
    // unreachable
    puts("Press Any Key To Continue.");
    getchar();
    return 0;
}
