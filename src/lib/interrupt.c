#include "interrupt.h"
#include "syscall.h"
#include "register.h"
#include "stdio.h"

static size_t INTERVAL = 1e5;
static size_t TICKS = 0;

Context *breakpoint(Context *context);

Context *tick(Context* context);

Context *handle_interrupt(Context *context, size_t scause, size_t stval) {
    switch (scause) {
        // instruction access fault
        case 1:{
            printf("Instruction Access Fault!\n");
            printf("sepc: 0x%x\n", context->sepc);
            printf("[Shutdown!]\n");
            shutdown();
        }
        // breakpoint
        case 3: {
            return breakpoint(context);
        }
        case 5: {
            size_t sip = register_read_sip();
            if(sip & REGISTER_SIP_STIE){
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
            return syscall(context);
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


