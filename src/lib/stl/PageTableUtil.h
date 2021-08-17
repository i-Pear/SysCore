#ifndef OS_RISC_V_PAGETABLEUTIL_H
#define OS_RISC_V_PAGETABLEUTIL_H

#include "stl.h"
#include "../../kernel/memory/memory.h"
#include "../../kernel/register.h"
#include "../../vdso/vdso.h"

extern size_t fast_syscall_page;

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
    static void init_pageTable(size_t page_table_base){
        // 0x3800_1000 -> 0x3800_1000 (4K)
        PageTableUtil::CreateMapping(page_table_base,
                                     0x38001000,
                                     0x38001000,
                                     PAGE_TABLE_LEVEL::SMALL,
                                     PRIVILEGE_LEVEL::SUPERVISOR);
        // 0x4000_0000 -> 0c4000_0000 (1G)
        PageTableUtil::CreateMapping(page_table_base,
                                     0x40000000,
                                     0x40000000,
                                     PAGE_TABLE_LEVEL::LARGE,
                                     PRIVILEGE_LEVEL::SUPERVISOR);
        // 0x8000_0000 -> 0x8000_0000 (1G)
        PageTableUtil::CreateMapping(page_table_base,
                                     0x80000000,
                                     0x80000000,
                                     PAGE_TABLE_LEVEL::LARGE,
                                     PRIVILEGE_LEVEL::USER);
        // add fast_syscall page
        PageTableUtil::CreateMapping(page_table_base,
                                     0x200000000,
                                     fast_syscall_page,
                                     PAGE_TABLE_LEVEL::SMALL,
                                     PRIVILEGE_LEVEL::USER);
        // map vdso
        PageTableUtil::CreateMapping(page_table_base,
                                     0x300000000,
                                     vdso_text_page,
                                     PAGE_TABLE_LEVEL::SMALL,
                                     PRIVILEGE_LEVEL::USER);
        PageTableUtil::CreateMapping(page_table_base,
                                     0x300001000,
                                     vdso_data_page,
                                     PAGE_TABLE_LEVEL::SMALL,
                                     PRIVILEGE_LEVEL::USER);
    }

    static void CreateMapping(size_t table_base,
                              size_t virtual_address,
                              size_t physical_address,
                              PAGE_TABLE_LEVEL page_table_level,
                              PRIVILEGE_LEVEL privilege_level){
        check_null(virtual_address, physical_address);
        check_table_base(table_base);
        if(page_table_level == PAGE_TABLE_LEVEL::MIDDLE){
            panic('un supported page_table_level, please use level small or large')
        }
        size_t ppn1 = get_ppn1(virtual_address);
        size_t ppn2 = get_ppn2(virtual_address);
        size_t ppn3 = get_ppn3(virtual_address);

        size_t* pte1 = (size_t *)table_base + ppn1;
        if(*pte1 == 0){
            if(page_table_level == PAGE_TABLE_LEVEL::LARGE){
                *pte1 = calc_ppn(physical_address) | leaf_attributes(privilege_level);
                return;
            }
            table_base = GetClearPage();
            *pte1 = calc_ppn(table_base) | non_leaf_attributes(privilege_level);
        }else{
            if(is_leaf(*pte1)){
                printf("PageTable Error! The entry 0x%x is used\n", *pte1);
                panic("")
            }
            table_base = ((*pte1 >> 10) << 12);
        }

        size_t* pte2 = (size_t*)table_base + ppn2;
        if(*pte2 == 0){
            table_base = GetClearPage();
            *pte2 = calc_ppn(table_base) | non_leaf_attributes(privilege_level);
        }else{
            if(is_leaf(*pte2)){
                printf("PageTable Error! The entry 0x%x is used\n", *pte2);
                panic("")
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

    static void FlushInstructionStream(){
        asm volatile("fence.i");
    }

    static size_t CalculateTableBaseBySatp(size_t satp){
        return satp << 12LL;
    }

    static void ClearPage(size_t page){
        memset((char *) page, 0, 4096);
    }

    static size_t GetClearPage(){
        size_t res = alloc_page();
        ClearPage(res);
        return res;
    }

    static void ErasePageTable(size_t page_table_base, bool release_physical_page = false) {
        check_table_base(page_table_base);

        if (page_table_base == CalculateTableBaseBySatp(register_read_satp())) {
            panic("You can't release an used page table");
        }

        for (int i = 0;i < 512; i++) {
            size_t* entry = (size_t *) page_table_base + i;
            if (*entry != 0) {
                erase_entry(entry, release_physical_page);
            }
        }

        if (is_page_alloced(page_table_base)) {
            dealloc_page(page_table_base);
            printf("[PageTable] dealloc 0x%x\n", page_table_base);
        }
    }

    static void EraseEntry(size_t page_table_base, size_t virtual_address, bool release_physical_page = false) {
        check_table_base(page_table_base);
        check_null(virtual_address, virtual_address);

        auto table_base = (size_t *) page_table_base;
        size_t ppn1 = get_ppn1(virtual_address);
        size_t ppn2 = get_ppn2(virtual_address);
        size_t ppn3 = get_ppn3(virtual_address);

        if (inner_erase_entry(table_base, ppn1, release_physical_page)) return;
        if (inner_erase_entry(table_base, ppn2, release_physical_page)) return;
        if (inner_erase_entry(table_base, ppn3, release_physical_page)) return;
    }
private:
    static bool inner_erase_entry(size_t*& table_base, size_t ppn, bool release_physical_page = false){
        size_t entry = *(table_base + ppn);
        size_t page = (entry >> 10) << 12;
        if (is_leaf(entry)) {
            if (release_physical_page && is_page_alloced(page)) {
                printf("[PageTable] dealloc 0x%x\n", page);
                dealloc_page(page);
            }
            *(table_base + ppn) = 0;
            return true;
        } else {
            table_base = (size_t *) page;
            return false;
        }
    }


    static void erase_entry(size_t* entry_address, bool release_physical_page = false) {
        size_t entry = *entry_address;
        size_t page = ((entry >> 10) << 12);
        if (is_leaf(entry)) {
            if (release_physical_page && is_page_alloced(page)) {
                printf("[PageTable] dealloc 0x%x\n", page);
                dealloc_page(page);
            }
            // clear page table entry
            *entry_address = 0;
        } else {
            ErasePageTable(page);
        }
    }

    // mark non leaf node as USER level, because user & kernel can access same non leaf entry
    static size_t non_leaf_attributes(PRIVILEGE_LEVEL level){
        return 0xd1;
//        if(level == PRIVILEGE_LEVEL::SUPERVISOR){
//            return 0xc1;
//        }else if(level == PRIVILEGE_LEVEL::USER){
//            return 0xd1;
//        }
//        return 0xd1;
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

    // if RWX is zero, it is non leaf node, otherwise it is leaf node
    static bool is_leaf(size_t pte){
        size_t mask = 7LL << 1LL;
        return is_efficient(pte) && (((pte & mask) ^ mask)!= mask);
    }

    // if V not zero, it is efficient
    static bool is_efficient(size_t pte){
        return pte & 1LL;
    }

    static void check_null(size_t virtual_address, size_t physical_address){
        if(virtual_address >= 0 && virtual_address < 0x1000){
            printf("[null pointer exception] virtual address 0x%x -> 0x%x\n", virtual_address, physical_address);
            panic("");
        }
    }

    static void check_table_base(size_t table_base){
        if(((table_base & 7LL) ^ 7LL) != 7LL){
            printf("table base = 0x%x\n", table_base);
            panic("table base must be 4K aligned")
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
