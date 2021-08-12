#ifndef SYSCORE_MMAP_CONTROL_H
#define SYSCORE_MMAP_CONTROL_H

#include "memory_keeper.h"
#include "memory.h"
#include "../../lib/stl/stl.h"
#include "../../lib/stl/PageTableUtil.h"
#include "../../lib/stl/list.h"

#define MMAP_VIRT_BEGIN 0x100000000

struct mmap_unit{
    size_t mapped_to;
    size_t source_addr;
};

class MmapControl {
private:

    MmapControl &operator=(const MmapControl &other) {}

    MmapControl(const MmapControl &other) {}

public:

    List<mmap_unit> memKeeper;

    size_t pageTable;

    size_t mmap_start=MMAP_VIRT_BEGIN;

    explicit MmapControl(size_t pageTable) : pageTable(pageTable) {}

    MmapControl(const MmapControl &other, size_t pageTable) : pageTable(pageTable) {

        mmap_start=other.mmap_start;

        // copy pages
        for (auto i=other.memKeeper.start;i;i=i->next) {
            size_t new_page = alloc_page(4096);
            memcpy((void *) new_page, (const void*)(i->data.source_addr), 4096);

            memKeeper.push_back({new_page,i->data.mapped_to});
            PageTableUtil::CreateMapping(
                    pageTable,
                    i->data.mapped_to,
                    new_page,
                    PAGE_TABLE_LEVEL::SMALL,
                    PRIVILEGE_LEVEL::USER);
        }

    }

    size_t mmap(size_t length) {
        size_t need_page=(length+4096-1)/4096; //align
        size_t ret=mmap_start;
        for(int i=0;i<need_page;i++){
            size_t new_page = alloc_page(4096);
            memset((void*)(new_page), 0, 4096);
            memKeeper.push_back({mmap_start,new_page});
            PageTableUtil::CreateMapping(
                    pageTable,
                    mmap_start,
                    new_page,
                    PAGE_TABLE_LEVEL::SMALL,
                    PRIVILEGE_LEVEL::USER);
            mmap_start+=4096;
        }
        PageTableUtil::FlushCurrentPageTable();

        return ret;
    }

    int munmap(size_t addr,size_t length){
        if(addr%0x1000!=0){
            panic("munmap address not aligned!")
        }
        for(size_t p=addr;p<addr+length;p+=4096){
            // free
            for(auto i=memKeeper.start;i;i=i->next){
                if(i->data.mapped_to==p){
                    dealloc_page(i->data.source_addr);
                    //todo: free page table
                    memKeeper.erase(i);
                    goto munmap_next;
                }
            }
            panic("munmap address is not mapped!")
            munmap_next:;
        }
        PageTableUtil::FlushCurrentPageTable();
        return 0;
    }

};

#endif //SYSCORE_MMAP_CONTROL_H
