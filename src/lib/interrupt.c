#include "interrupt.h"
#include "syscall.h"
#include "register.h"
#include "stl.h"

static size_t INTERVAL = 1e5;
static size_t TICKS = 0;
KernelContext kernelContext;

Context *breakpoint(Context *context);

Context *tick(Context* context);

Context *handle_interrupt(Context *context, size_t scause, size_t stval) {
    int is_interrupt = (int)(scause >> 63);
    scause &= 31;
    switch (scause) {
        case 1:{
            if(is_interrupt == 0){
                // load ins fault
                printf("load ins fault\n");
                shutdown();
            }else{
                printf("s-mode software interrupt\n");
                shutdown();
            }
            break;
        }
        // breakpoint
        case 3: {
            return breakpoint(context);
        }
        case 5: {
            if(is_interrupt){
                // supervisor-level timer interrupt
                return tick(context);
            }else{
                // load access fault
                printf("Load Access Fault\n");
                printf("Perhaps Page is Error\n");
                printf("sepc: 0x%x\n", context->sepc);
                printf("[Shutdown!]\n");
                shutdown();
            }
        }
        // user ecall
        case 8:{
            Context *ret = syscall(context);
            return ret;
        }
        default: {
            printf("scause: %d\n", scause);
            printf("sepc: 0x%x\n", context->sepc);
            printf("Interrupt scause wrong, shutdown!.\n");
            shutdown();
        }
    }
    return NULL;
}

Context *breakpoint(Context *context) {
    printf("Breakpoint at 0x%x\n", context->sepc);
    // 2 or 4 ?
    context->sepc += 4;
    return context;
}

Context *tick(Context* context) {
    set_next_timeout();
    TICKS += 1;
    if (TICKS % 100 == 0 && TICKS <= 1000) {
        printf("[TICK] %d tick\n", TICKS);
    }
    return context;
}

void set_next_timeout() {
    set_timer(read_time() + INTERVAL);
}

