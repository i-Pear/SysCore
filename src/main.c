#include "lib/page.h"
#include "lib/interrupt.h"
#include "lib/register.h"
#include "lib/elf_data.h"
#include "lib/elf_loader.h"

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
    // load ELF
    int size=sizeof(ELF_DATA);
    size_t ptr=load_elf(ELF_DATA,size);

    printf("[DEBUG] Prepare For User Mode.\n");
    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    /**
     * 用户栈
     * 由于alloc_page函数返回值为一页的物理首地址，故此处增加__kernel_vir_offset
     * 栈通常向低地址方向增长，故此处增加__page_size
     */
    thread_context.sp = (size_t) alloc_page() + __page_size + __kernel_vir_offset;
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
    size_t user_satp = (size_t) alloc_page();

    for (int i = 0; i < 511; i++) {
        *((size_t *)user_satp + i) = 0;
    }

    // 0x8000_0000 -> 0x8000_0000
    // TODO 此处不合理
    *((size_t *)user_satp + 2) = (0x80000 << 10) | 0xdf;
    // 0xffff_ffff_8000_0000 -> 0x8000_0000
    *((size_t *)user_satp + 510) = (0x80000 << 10) | 0xdf;
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
    printf("[DEBUG] Prepare For SuperVisor Mode With VirtualOffset.\n");
    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    thread_context.sp = register_read_sp() + __kernel_vir_offset;
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
    thread_context.sepc = (size_t) init_thread + __kernel_vir_offset;
    thread_context.ra = (size_t) init_thread + __kernel_vir_offset;
    thread_context.satp = kernelContext.kernel_satp;
    __restore(&thread_context);
}

void print_sp() {
    printf("sp = 0x%x\n", register_read_sp());
}

void print_satp() {
    printf("satp = 0x%x\n", register_read_satp());
}

int main(size_t hart_id, size_t dtb_pa) {
    printf("[DEBUG] Memory Init.\n");
    memory_init();
    puts("[DEBUG] Interrupt & Timer Interrupt Open.");
    kernelContext.kernel_satp = register_read_satp();
    interrupt_timer_init();

//    init_thread();
    turn_to_virtual_supervisor_mode();
    // unreachable
    puts("Press Any Key To Continue.");
    getchar();
    return 0;
}
