#include "syscall.h"
#include "register.h"
#include "stl.h"
#include "scheduler.h"

#define return(x) context->a0=x

Context *syscall(Context *context) {
    // Check SystemCall Number
    // printf("[SYSCALL] call id=%d\n",context->a7);
    switch (context->a7) {
        // Print Register
        // @param: x: register number
        case 0: {
            printf("[DEBUG] x%d = %d\n", context->a0, *((size_t *) context + context->a0));
            break;
        }
        case 1: {
            printf("[DEBUG] satp = 0x%x\n", context->satp);
            break;
        }
        case SYS_write: {
            int file = context->a0;
            char *buf = context->a1 + get_running_elf_page();
            int count = context->a2;
            if (file == 1) {
                // stdout
                for (int i = 0; i < count; i++)putchar(buf[i]);
                return(count);
            } else if (file == 2) {
                // stderr
                for (int i = 0; i < count; i++)putchar(buf[i]);
                return(count);
            } else {
                // Other files
            }
            break;
        }
        case SYS_exit: {
            exit_process();
            break;
        }
        case SYS_getpid: {
            return(get_running_pid());
            break;
        }
        case SYS_uname: {
            struct utsname *required_uname = context->a0 + get_running_elf_page();
            memcpy(required_uname, &uname, sizeof(uname));
            return(0);
            break;
        }
        case SYS_wait4:{

            break;
        }
        case SYS_sched_yield: {
            return(0);
            yield();
            break;
        }
        default: {
            printf("[SYSCALL] Unhandled Syscall: %d\n", context->a7);
            // shutdown();
        }
    }
    context->sepc += 4;
    return context;
}