#ifndef OS_RISC_V_BRK_CONTROL_H
#define OS_RISC_V_BRK_CONTROL_H

#include "memory_keeper.h"
#include "memory.h"
#include "../../lib/stl/stl.h"
#include "../../lib/stl/PageTableUtil.h"

#define BRK_VIRT_BEGIN 0xa0000000

class BrkControl {
private:

    BrkControl &operator=(const BrkControl &other) {}

    BrkControl(const BrkControl &other) {}

public:

    MemKeeper memKeeper;

    size_t pageTable;

    bool brk_init;
    size_t page_count;

    BrkControl(size_t pageTable) : pageTable(pageTable) {
        brk_init = false;
        page_count = 0;
    }

    BrkControl(const BrkControl &other, size_t pageTable) : pageTable(pageTable) {
        if (!other.brk_init) {
            brk_init = false;
            page_count = 0;
        } else {
            // copy pages
            brk_init = true;
            page_count = other.page_count;

            for (int i = 0; i < page_count; i++) {
                size_t new_page = alloc_page(4096);
                memcpy((void *) new_page, memKeeper.elements[i], 4096);

                memKeeper.add((void *) new_page);
                PageTableUtil::CreateMapping(
                        pageTable,
                        BRK_VIRT_BEGIN + 4096 * i,
                        new_page + 4096 * i,
                        PAGE_TABLE_LEVEL::SMALL,
                        PRIVILEGE_LEVEL::USER);
            }
        }
    }

    size_t brk(size_t p) {
        if (!brk_init) {
            size_t page = alloc_page(4096);
            page_count = 1;
            memKeeper.add(reinterpret_cast<void *>(page));
            PageTableUtil::CreateMapping(pageTable, BRK_VIRT_BEGIN, page,
                                         PAGE_TABLE_LEVEL::SMALL,
                                         PRIVILEGE_LEVEL::USER);
            PageTableUtil::FlushCurrentPageTable();
        }
        if (p == 0) {
            return BRK_VIRT_BEGIN;
        }
        size_t need_space = p - BRK_VIRT_BEGIN;
        int need_page = (need_space + 4096 - 1) / 4096;
        if (need_space < 0 || need_page > 2000) {
            panic("brk failed")
        }
        if (need_page > page_count) {
            for (int i = page_count; i < need_page; i++) {
                size_t new_page = alloc_page(4096);
                memKeeper.add(reinterpret_cast<void *>(new_page));
                PageTableUtil::CreateMapping(
                        pageTable,
                        BRK_VIRT_BEGIN + 4096 * i,
                        new_page + 4096 * i,
                        PAGE_TABLE_LEVEL::SMALL,
                        PRIVILEGE_LEVEL::USER);
            }
            page_count = need_page;
        }
        PageTableUtil::FlushCurrentPageTable();

        return p;
    }

};

#endif //OS_RISC_V_BRK_CONTROL_H
