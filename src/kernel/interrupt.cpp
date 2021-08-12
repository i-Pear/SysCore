#include "interrupt.h"
#include "syscall.h"
#include "register.h"
#include "../lib/stl/stl.h"
#include "../lib/stl/PageTableUtil.h"
#include "scheduler.h"

static size_t INTERVAL = 1e5;
static size_t TICKS = 0;
KernelContext kernelContext;

Context *breakpoint(Context *context);

Context *tick(Context* context);

Context *page_fault(Context* context, size_t stval);

Context *handle_interrupt(Context *context, size_t scause, size_t stval) {
    int is_interrupt = (int)(scause >> 63);
    scause &= 31;
    switch (scause) {
        case 1:{
            if(is_interrupt == 0){
                // load ins fault
                mtl(">>>>>> load ins fault <<<<<<");
                page_fault(context, stval);
                __restore();
            }else{
                printf("s-mode software interrupt\n");
                shutdown();
            }
            break;
        }
        case 2:{
            if(is_interrupt == 0){
                panic("illegal ins")
            }
            else{
                printf("unknown interrupt\n");
                shutdown();
            }
            break;
        }
        // breakpoint
        case 3: {
            breakpoint(context);
            __restore();
        }
        case 5: {
            if(is_interrupt){
                // supervisor-level timer interrupt
                tick(context);
                __restore();
            }else{
                // load access fault
                mtl(">>>>>> Load Access Fault <<<<<<");
//                printf("Perhaps Page is Error\n");
//                printf("sepc: 0x%x\n", context->sepc);
//                printf("[Shutdown!]\n");
//                shutdown();
                page_fault(context, stval);
                __restore();
            }
        }
        case 7:{
            if(is_interrupt == 0){
                // store access fault
                mtl(">>>>>> store access fault <<<<<<");
                page_fault(context, stval);
                __restore();
            }else{
                printf("machine timer interrupt interrupt\n");
                shutdown();
            }
            break;
        }
        // user ecall
        case 8:{
            syscall(context);
            __restore();
        }
        // ins page fault & load page fault
        case 12 ... 13:{
            page_fault(context, stval);
            __restore();
        }
        // store page fault
        case 15:{
            page_fault(context, stval);
            __restore();
        }
        default: {
            printf("scause: %d\n", scause);
            printf("sepc: 0x%x\n", context->sepc);
            printf("Interrupt scause wrong, shutdown!.\n");
            shutdown();
        }
    }
    return NULL; // unreachable
}

Context *page_fault(Context* context, size_t stval){
    size_t satp = context->satp;

    size_t vir_addr = stval;
    size_t table_base = satp << 12;

    size_t* vir= reinterpret_cast<size_t *>(vir_addr);
    printf("[Page fault] trying to access address 0x%x  pc=0x%x\n",vir_addr,context->sepc);
    panic("why page fault?")

//    PageTableUtil::CreateMapping(table_base, vir_addr, phy_addr, PAGE_TABLE_LEVEL::SMALL, PRIVILEGE_LEVEL::USER);
    return context;
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
