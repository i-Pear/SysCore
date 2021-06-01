#include "syscall.h"
#include "register.h"
#include "../lib/stl/stl.h"
#include "scheduler.h"
#include "external_structs.h"
#include "times.h"

size_t get_actual_page(size_t x){
    return x > 0x80000000?x:x+ get_running_elf_page();
}

Context *syscall(Context *context) {
    // Check SystemCall Number
    // printf("[SYSCALL] call id=%d\n",context->a7);
    switch (context->a7) {
        // Print Register
        // @param: x: register number
        case SYS_getchar: {
            context->a0 = getchar_blocked();
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
            char* buf=(char*)get_actual_page(context->a1);
            int count=context->a2;
            for(int i=0;i<count;i++){
                putchar(buf[i]);
            }
            context->a0 = count;
            break;
        }
        case SYS_execve: {
            execute((char*)get_actual_page(context->a0));
            break;
        }
        case SYS_gettimeofday: {
            get_timespec((TimeVal*)get_actual_page(context->a0));
            time_seconds++;
            context->a0 = 0;
            break;
        }
        case SYS_exit: {
            exit_process(context->a0);
            break;
        }
        case SYS_getppid: { lty(get_running_ppid());
            context->a0 = get_running_ppid();
            break;
        }
        case SYS_getpid: { lty(get_running_pid());
            context->a0 = get_running_pid();
            break;
        }
        case SYS_times: {
            struct ES_tms *tms = (ES_tms*)get_actual_page(context->a0);
            tms->tms_utime = 1;
            tms->tms_stime = 1;
            tms->tms_cutime = 1;
            tms->tms_cstime = 1;
            context->a0 = 1000;
            break;
        }
        case SYS_uname: {
            struct ES_utsname *required_uname = (ES_utsname *)get_actual_page(context->a0);
            memcpy(required_uname, &ES_uname, sizeof(ES_uname));
            context->a0 = 0;
            break;
        }
        case SYS_wait4: {
            context->a0 = wait((int*)get_actual_page(context->a1));
            break;
        }
        case SYS_nanosleep: {
            TimeVal *timeVal = (TimeVal *)get_actual_page(context->a0);
            time_seconds += timeVal->sec;
            time_macro_seconds += timeVal->usec;
            context->a0 = 0;
            break;
        }
        case SYS_clone: {
            clone(context->a0, context->a1, context->a2);
            break;
        }
        case SYS_sched_yield: {
            context->a0 = 0;
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