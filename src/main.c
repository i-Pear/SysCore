#include <page.h>
#include <interrupt.h>

void D(size_t x) { printf("0x%x\n", x); }

void daemon_thread() {
    puts("[Halt] Program Halt In U Mode");
    while (1);
}

void init_thread() {
    Context thread_context;
    __asm__ volatile(
    "csrr %0, sstatus"
    :"=r"(thread_context.sstatus)
    );
    // kernel stack
    __asm__ volatile(
    "mv %0, sp"
    :"=r"(thread_context.x[2])
    );
    thread_context.sstatus |= 0x120; // spie = 1 && spp = 1
    thread_context.sepc = (size_t) daemon_thread;
    thread_context.sepc += __kernel_vir_offset;
    thread_context.x[1] = (size_t) daemon_thread;
    // interrupt_timer_init();
    __restore(&thread_context);
}


int main(size_t hart_id, size_t dtb_pa) {
    puts("[Memory] Initializing...");
    memory_init();

    interrupt_timer_init();

    puts("[Memory Map] Initializing...");
    memory_map_init();

    init_thread();
    // unreachable
    puts("Press Any Key To Continue.");
    getchar();
    return 0;
}
