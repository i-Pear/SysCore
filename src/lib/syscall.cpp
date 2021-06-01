#include "syscall.h"
#include "register.h"
#include "stl.h"
#include "scheduler.h"
#include "external_structs.h"
#include "times.h"

#define return(x) context->a0=x
#define get_actual_page(x) ((x>0x80000000)?x:x+ get_running_elf_page())


// 0;
#define NOP(a) 0

Context *syscall(Context *context) {
    // Check SystemCall Number
    // printf("[SYSCALL] call id=%d\n",context->a7);
    switch (context->a7) {
        // Print Register
        // @param: x: register number
        case SYS_getchar: {
            return(getchar_blocked());
            break;
        }
        case 0: {
            printf("[DEBUG] x%d = %d\n", context->a0, *((size_t *) context + context->a0));
            break;
        }
        case 1: {
            printf("[DEBUG] satp = 0x%x\n", context->satp);
            break;
        }
        case SYS_write:{
            int fd=context->a0;
            assert(fd==1);
            char* buf=get_actual_page(context->a1);
            int count=context->a2;
            for(int i=0;i<count;i++){
                putchar(buf[i]);
            }
            return(count);
            break;
        }
        case SYS_execve: {
            execute(get_actual_page(context->a0));
            break;
        }
        case SYS_gettimeofday: {
            get_timespec(get_actual_page(context->a0));
            time_seconds++;
            return(0);
            break;
        }
        case SYS_exit: {
            exit_process(context->a0);
            break;
        }
        case SYS_getppid: { lty(get_running_ppid());
            return(get_running_ppid());
            break;
        }
        case SYS_getpid: { lty(get_running_pid());
            return(get_running_pid());
            break;
        }
        case SYS_times: {
            struct ES_tms *tms = get_actual_page(context->a0);
            tms->tms_utime = 1;
            tms->tms_stime = 1;
            tms->tms_cutime = 1;
            tms->tms_cstime = 1;
            return(1000);
            break;
        }
        case SYS_uname: {
            struct ES_utsname *required_uname = get_actual_page(context->a0);
            memcpy(required_uname, &ES_uname, sizeof(ES_uname));
            return(0);
            break;
        }
        case SYS_wait4: {
            return(wait(get_actual_page(context->a1)));
            break;
        }
        case SYS_nanosleep: {
            TimeVal *timeVal = get_actual_page(context->a0);
            time_seconds += timeVal->sec;
            time_macro_seconds += timeVal->usec;
            return(0);
            break;
        }
        case SYS_clone: {
            clone(context->a0, context->a1, context->a2);
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