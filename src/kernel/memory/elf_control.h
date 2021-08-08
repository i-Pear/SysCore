#ifndef OS_RISC_V_ELF_CONTROL_H
#define OS_RISC_V_ELF_CONTROL_H

#include "../../lib/stl/vector.h"
#include "../../lib/stl/list.h"
#include "memory.h"
#include "../../lib/stl/PageTableUtil.h"
#include "../../lib/stl/RefCountPtr.h"

struct elf_ctl_page{
    size_t v_addr;
    size_t m_addr;
};

extern bool chh;

/**
 * manages an elf_segment: contains many pages
 */
class elf_ctl_segment{
public:

    bool _is_text;
    Vector<elf_ctl_page> pages;

    explicit elf_ctl_segment(bool is_text):_is_text(is_text){}

    void add(size_t page_table,size_t target_v_addr,size_t source_mem_addr){
        size_t page= alloc_page(4096);
        memcpy((void *)page, (void *)source_mem_addr, 4096);
        PageTableUtil::CreateMapping(page_table,
                                     target_v_addr,
                                     page,
                                     PAGE_TABLE_LEVEL::SMALL,
                                     PRIVILEGE_LEVEL::USER);
        pages.push_back({target_v_addr,page});

        if(chh){
            // check sum
            size_t res=0;
            for(char* c=(char*)page;c<(char*)(page+4096);c++){
                res=(res*10007+*c)%1000000007;
            }
            printf("vaddr=0x%x - 0x%x   checksum: 0x%x\n",target_v_addr,target_v_addr+4096,res);
        }
    }

    bool is_text() const{
        return _is_text;
    }

    elf_ctl_segment(const elf_ctl_segment& other,size_t page_table):_is_text(other._is_text){
        // copy all pages
        int length=other.pages.length();
        for(int i=0;i<length;i++){
            elf_ctl_page& cnt=other.pages[i];
            add(page_table,cnt.v_addr,cnt.m_addr);
        }
    }

    ~elf_ctl_segment(){
        int length=pages.length();
        for(int i=0;i<length;i++){
            dealloc_page(pages[i].m_addr);
        }
    }
};

/**
 * manages all elf memory of a process: contains many segments
 */
class Elf_Control{
private:
    // copying is not permitted
    Elf_Control& operator=(const Elf_Control& other)=default;
    Elf_Control(const Elf_Control& other)=default;

public:

    size_t page_table;
    RefCountPtr<elf_ctl_segment> segments[2]; // [0] is text; [1] is data

    explicit Elf_Control(size_t page_table):page_table(page_table){}

    Elf_Control(const Elf_Control& other,size_t page_table):page_table(page_table){
        segments[0]=other.segments[0];
        segments[1]=RefCountPtr<elf_ctl_segment>(
                new elf_ctl_segment(*other.segments[1],page_table)
        );
    }

    void init_segments(){
        segments[0]=RefCountPtr<elf_ctl_segment>(new elf_ctl_segment(true));
        segments[1]=RefCountPtr<elf_ctl_segment>(new elf_ctl_segment(false));
    }

    void bind_text_page(size_t target_v_addr,size_t source_mem_addr){
        chh= false;
        segments[0]->add(page_table,target_v_addr,source_mem_addr);
    }

    void bind_data_page(size_t target_v_addr,size_t source_mem_addr){
        chh= true;
        segments[1]->add(page_table,target_v_addr,source_mem_addr);
    }
};

#endif //OS_RISC_V_ELF_CONTROL_H
