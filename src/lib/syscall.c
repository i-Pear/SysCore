#include <syscall.h>

Context* syscall(Context* context){
    // Check SystemCall Number
    switch (context->x[7]) {
        // Print Register
        // @param: x: register number
        case 0:{
            printf("[DEBUG] x%d = %d\n", context->x[10], context->x[context->x[10]]);
            break;
        }
        case SYS_getpid:{
            context->x[10] = 0;
            break;
        }
        default:{
            printf("[SYSCALL] Unhandled Syscall: %d\n", context->x[10]);
            shutdown();
        }
    }
    return context;
}