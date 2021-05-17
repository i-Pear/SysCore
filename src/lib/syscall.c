#include "syscall.h"
#include "register.h"

Context* syscall(Context* context){
    // Check SystemCall Number
    switch (context->a7) {
        // Print Register
        // @param: x: register number
        case 0:{
            printf("[DEBUG] x%d = %d\n", context->a0, *((size_t *)context + context->a0));
            break;
        }
        case 1:{
            printf("[DEBUG] satp = 0x%x\n", context->satp);
            break;
        }
        case SYS_getpid:{
            context->a0 = 0;
            break;
        }
        default:{
            printf("[SYSCALL] Unhandled Syscall: %d\n", context->a0);
            shutdown();
        }
    }
    context->sepc += 4;
    return context;
}