#include <page.h>
#include <interrupt.h>
#include <register.h>

void D(size_t x) { printf("0x%x\n", x); }


void daemon_thread() {
//    asm volatile ("li a7, 8");
//    asm volatile ("ecall");

    asm volatile ("li a7, 0");
    asm volatile ("li a1, 999");
    asm volatile ("li a0, 11");
    asm volatile ("ecall");

    // 20cf2
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
 *
 * 另外我们还得给u-mode分配页表，因为s态和u态无法使用一个页表
 */
void init_thread() {
    Context thread_context;
    thread_context.sstatus = register_read_sstatus();
    // user stack
    size_t user_stack_page = (size_t)alloc_page();
    thread_context.x[2] = user_stack_page + __page_size;
    thread_context.x[2] += __kernel_vir_offset;
    thread_context.sstatus |= REGISTER_SSTATUS_SPP; // spp = 1
    thread_context.sstatus ^= REGISTER_SSTATUS_SPP; // spp = 0
    thread_context.sstatus |= REGISTER_SSTATUS_SPIE; // spie = 1
    thread_context.sepc = (size_t) daemon_thread;
    thread_context.sepc += __kernel_vir_offset;
    thread_context.x[1] = (size_t) daemon_thread;
    thread_context.x[1] += __kernel_vir_offset;

    size_t init_thread_page_table = (size_t)alloc_page();
    // calculate satp
    thread_context.satp = init_thread_page_table;
    thread_context.satp >>= 12;
    thread_context.satp |= (8 << 60);
    make_map(memory_map, (size_t *)init_thread_page_table, thread_context.sepc & 0xfffffffffffff000, (size_t) daemon_thread & 0xfffffffffffff000);
    make_map(memory_map, (size_t *)init_thread_page_table, user_stack_page + __kernel_vir_offset, user_stack_page);
    size_t real_addr = (size_t)__interrupt;
    make_map(memory_map, (size_t *)init_thread_page_table, (real_addr + __kernel_vir_offset) & 0xfffffffffffff000, real_addr & 0xfffffffffffff000);
    make_map(memory_map, (size_t *)init_thread_page_table, ((real_addr + __kernel_vir_offset) & 0xfffffffffffff000) + 0x1000, (real_addr & 0xfffffffffffff000) + 0x1000);

    puts("[DEBUG] Timer Interrupt Start.");
    interrupt_timer_init();

    __turn_to_user_mode(&thread_context);
}


int main(size_t hart_id, size_t dtb_pa) {
    puts("[Memory] Initializing...");
    memory_init();

    init_thread();
    // unreachable
    puts("Press Any Key To Continue.");
    getchar();
    return 0;
}
