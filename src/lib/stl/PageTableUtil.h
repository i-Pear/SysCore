#ifndef OS_RISC_V_PAGETABLEUTIL_H
#define OS_RISC_V_PAGETABLEUTIL_H

#include "stl.h"
#include "../../kernel/memory/memory.h"

// 页面大小
enum class PAGE_TABLE_LEVEL{
    LARGE, // 1G
    MIDDLE, // 2M
    SMALL // 4K
};

// 特权级
enum class PRIVILEGE_LEVEL{
    SUPERVISOR,
    USER
};

class PageTableUtil{
public:
    static void CreateMapping(size_t table_base,
                              size_t virtual_address,
                              size_t physical_address,
                              PAGE_TABLE_LEVEL page_table_level,
                              PRIVILEGE_LEVEL privilege_level){
        check_null(virtual_address, physical_address);
        size_t ppn1 = get_ppn1(virtual_address);
        size_t ppn2 = get_ppn2(virtual_address);
        size_t ppn3 = get_ppn3(virtual_address);

        size_t* pte1 = (size_t *)table_base + ppn1;
        if(*pte1 == 0){
            if(page_table_level == PAGE_TABLE_LEVEL::LARGE){
                *pte1 = calc_ppn(physical_address) | leaf_attributes(privilege_level);
                return;
            }
            size_t new_addr = alloc_page(4096);
            memset((char *) new_addr, 0, 4096);
            table_base = new_addr;
            *pte1 = calc_ppn(new_addr) | non_leaf_attributes(privilege_level);
        }else{
            if(page_table_level == PAGE_TABLE_LEVEL::LARGE){
                return;
            }
            table_base = ((*pte1 >> 10) << 12);
        }

        size_t* pte2 = (size_t*)table_base + ppn2;
        if(*pte2 == 0){
            if(page_table_level == PAGE_TABLE_LEVEL::MIDDLE){
                *pte2 = calc_ppn(physical_address) | leaf_attributes(privilege_level);
                return;
            }
            size_t new_addr = alloc_page(4096);
            memset((char *) new_addr, 0, 4096);
            table_base = new_addr;
            *pte2 = calc_ppn(new_addr) | non_leaf_attributes(privilege_level);
        }else{
            if(page_table_level == PAGE_TABLE_LEVEL::MIDDLE){
                return;
            }
            table_base = ((*pte2 >> 10) << 12);
        }

        size_t* pte3 = (size_t*)table_base + ppn3;
        if(*pte3 == 0){
            *pte3 = calc_ppn(physical_address) | leaf_attributes(privilege_level);
        }
    }

    static void FlushCurrentPageTable(){
        asm volatile("sfence.vma zero, zero");
    }

private:
    static size_t non_leaf_attributes(PRIVILEGE_LEVEL level){
        if(level == PRIVILEGE_LEVEL::SUPERVISOR){
            return 0xc1;
        }else if(level == PRIVILEGE_LEVEL::USER){
            return 0xd1;
        }
        return 0xd1;
    }

    static size_t leaf_attributes(PRIVILEGE_LEVEL level){
        if(level == PRIVILEGE_LEVEL::SUPERVISOR){
            return 0xcf;
        }else if(level == PRIVILEGE_LEVEL::USER){
            return 0xdf;
        }
        return 0xdf;
    }

    static size_t calc_ppn(size_t physical_address){
        return ((physical_address >> 12) << 10);
    }

    static void check_null(size_t virtual_address, size_t physical_address){
        if(virtual_address == 0){
            printf("[null pointer exception] virtual address 0 -> 0x%x\n", physical_address);
            panic("");
        }
    }

    static size_t get_ppn1(size_t virtual_address){
        return (virtual_address & (0b111111111LL << 30)) >> 30;
    }

    static size_t get_ppn2(size_t virtual_address){
        return (virtual_address & (0b111111111LL << 21)) >> 21;
    }

    static size_t get_ppn3(size_t virtual_address){
        return (virtual_address & (0b111111111LL << 12)) >> 12;
    }


};

#endif //OS_RISC_V_PAGETABLEUTIL_H
