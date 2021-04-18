#include <interrupt.h>

static size_t INTERVAL = 1e5;
static size_t TICKS = 0;

Context *breakpoint(Context *context);

Context *tick(Context* context);

Context *handle_interrupt(Context *context, size_t scause, size_t stval) {
    switch (scause) {
        case 3: {
            return breakpoint(context);
        }
        case 5: {
            return tick(context);
        }
        default: {
            printf("scause: %d\n", scause);
            printf("sepc: 0x%x\n", context->sepc);
            printf("Interrupt scause wrong, exit.\n");
            shutdown();
        }
    }
    return NULL;
}

Context *breakpoint(Context *context) {
    printf("Breakpoint at 0x%x\n", context->sepc);
    context->sepc += 2;
    return context;
}

Context *tick(Context* context) {
    set_next_timeout();
    TICKS += 1;
    if (TICKS % 100 == 0) {
        printf("[TICK] %d tick\n", TICKS);
    }
    return context;
}

void set_next_timeout() {
    set_timer(read_time() + INTERVAL);
}

