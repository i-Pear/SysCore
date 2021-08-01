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

void fix_user_page_table(Context* context);

Context *handle_interrupt(Context *context, size_t scause, size_t stval) {
    fix_user_page_table(context);
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
//                mtl("illegal ins");
//                lty(context->sepc);
//                lty(context->stval);
//                lty(context->satp);
//
//                size_t satp = context->satp;
//
//                size_t vir_addr = context->sepc;
//                size_t table_base = satp << 12;
//                lty(vir_addr);
//                lty(table_base);
//
//                size_t ppn1 = (vir_addr & (0b111111111LL << 30)) >> 30;
//                size_t ppn2 = (vir_addr & (0b111111111LL << 21)) >> 21;
//                size_t ppn3 = (vir_addr & (0b111111111LL << 12)) >> 12;
//                size_t offset = (vir_addr & (0xfff));
//                lty(ppn1);
//                lty(ppn2);
//                lty(ppn3);
//                lty(offset);
//
//                size_t* pte1 = (size_t *)table_base + ppn1;
//                lty(*pte1);
//
//                if(*pte1 == 0){
//                    size_t new_addr = alloc_page(4096);
//                    memset((char *) new_addr, 0, 4096);
//                    table_base = new_addr;
//                    lty(new_addr);
//                    *pte1 = ((new_addr >> 12) << 10) | 0xd1;
//                    lty(*pte1);
//                }else{
//                    lty(*pte1);
//                    table_base = (*pte1 >> 10) << 12;
//                    lty(table_base);
//                }
//
//                size_t* pte2 = (size_t*)table_base + ppn2;
//                lty(*pte2);
//
//                if(*pte2 == 0){
//                    size_t new_addr = alloc_page(4096);
//                    memset((char *) new_addr, 0, 4096);
//                    table_base = new_addr;
//                    lty(new_addr);
//                    *pte2 = ((new_addr >> 12) << 10) | 0xd1;
//                    lty(*pte2);
//                }else{
//                    lty(*pte2);
//                    table_base = (*pte2 >> 10) << 12;
//                    lty(table_base);
//                }
//
//                size_t* pte3 = (size_t*)table_base + ppn3;
//                lty(*pte3);
//
//                if(*pte3 == 0){
////        size_t new_addr = elf_exec_page_base_only_one + ((vir_addr >> 12) << 12);
//                    size_t new_addr = alloc_page(4096);
//                    memset((char *) new_addr, 0, 4096);
//                    lty(new_addr);
//                    *pte3 = ((new_addr >> 12) << 10) | 0xdf;
//                    lty(*pte3);
//                }else{
//                    lty(*pte3);
//                    table_base = (*pte3 >> 10) << 12;
//                    lty(table_base);
//                }
//
//                lty(table_base + offset);
//                lty(*((size_t *)table_base + offset));

                shutdown();
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
    size_t phy_addr = get_running_elf_page() + ((vir_addr >> 12) << 12);

    PageTableUtil::CreateMapping(table_base, vir_addr, phy_addr, PAGE_TABLE_LEVEL::SMALL, PRIVILEGE_LEVEL::USER);

    size_t ppn1 = (vir_addr & (0b111111111LL << 30)) >> 30;
    size_t ppn2 = (vir_addr & (0b111111111LL << 21)) >> 21;
    size_t ppn3 = (vir_addr & (0b111111111LL << 12)) >> 12;

    size_t* pte1 = (size_t *)table_base + ppn1;
    if(*pte1 == 0){
        size_t new_addr = alloc_page(4096);
        memset((char *) new_addr, 0, 4096);
        table_base = new_addr;
        *pte1 = ((new_addr >> 12) << 10) | 0xd1;
    }else{
        table_base = ((*pte1 >> 10) << 12);
    }

    size_t* pte2 = (size_t*)table_base + ppn2;

    if(*pte2 == 0){
        size_t new_addr = alloc_page(4096);
        memset((char *) new_addr, 0, 4096);
        table_base = new_addr;
        *pte2 = ((new_addr >> 12) << 10) | 0xd1;
    }else{
        table_base = ((*pte2 >> 10) << 12);
    }

    size_t* pte3 = (size_t*)table_base + ppn3;

    if(*pte3 == 0){
        *pte3 = ((phy_addr >> 12) << 10) | 0xdf;
    }else{
        table_base = ((*pte3 >> 10) << 12);
    }

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

void fix_user_page_table(Context* context){
    size_t user_table_base = context->satp << 12;
    size_t kernel_table_base = register_read_satp() << 12;
    *(size_t*)kernel_table_base = *(size_t*)user_table_base;
    PageTableUtil::CreateMapping(kernel_table_base,
                                 0x38001000,
                                 0x38001000,
                                 PAGE_TABLE_LEVEL::SMALL,
                                 PRIVILEGE_LEVEL::SUPERVISOR);
    PageTableUtil::FlushCurrentPageTable();
}

