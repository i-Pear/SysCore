#include "syscall.h"
#include "register.h"
#include "stl.h"
#include "scheduler.h"
#include "external_structs.h"

#define return(x) context->a0=x

Context *syscall(Context *context) {
    lty(context);
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
            lty(context->a1);
            printf("[syscall write] buf=0x%x\n",buf);
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
        case SYS_getppid:{
            return(get_running_ppid());
            break;
        }
        case SYS_getpid: {
            return(get_running_pid());
            break;
        }
        case SYS_times:{
            struct ES_tms* tms=context->a0+ get_running_elf_page();
            tms->tms_utime=1;
            tms->tms_stime=1;
            tms->tms_cutime=1;
            tms->tms_cstime=1;
            return(1000);
            break;
        }
        case SYS_uname: {
            struct ES_utsname *required_uname = context->a0 + get_running_elf_page();
            memcpy(required_uname, &ES_uname, sizeof(ES_uname));
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