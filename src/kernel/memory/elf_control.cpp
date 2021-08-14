#include "elf_control.h"

elf_ctl_segment::elf_ctl_segment(bool is_text):_is_text(is_text){}

void elf_ctl_segment::add(size_t page_table,size_t target_v_addr,size_t source_mem_addr){
    size_t page= alloc_page(4096);
    memcpy((void *)page, (void *)source_mem_addr, 4096);
    PageTableUtil::CreateMapping(page_table,
                                 target_v_addr,
                                 page,
                                 PAGE_TABLE_LEVEL::SMALL,
                                 PRIVILEGE_LEVEL::USER);
    pages.push_back({target_v_addr,page});
}

bool elf_ctl_segment::is_text() const{
    return _is_text;
}

elf_ctl_segment::elf_ctl_segment(const elf_ctl_segment& other,size_t page_table):_is_text(other._is_text){
    // copy all pages
    int length=other.pages.length();
    for(int i=0;i<length;i++){
        elf_ctl_page& cnt=other.pages[i];
        add(page_table,cnt.v_addr,cnt.m_addr);
    }
}

void elf_ctl_segment::register_pageTable(size_t page_table){
    int length=pages.length();
    for(int i=0;i<length;i++){
        elf_ctl_page& cnt=pages[i];
        PageTableUtil::CreateMapping(page_table,
                                    cnt.v_addr,
                                    cnt.m_addr,
                                    PAGE_TABLE_LEVEL::SMALL,
                                    PRIVILEGE_LEVEL::USER);
    }
}

elf_ctl_segment::~elf_ctl_segment(){
    int length=pages.length();
    for(int i=0;i<length;i++){
        dealloc_page(pages[i].m_addr);
    }
}

Elf_Control::Elf_Control(size_t page_table):page_table(page_table){}

Elf_Control::Elf_Control(const Elf_Control& other,size_t page_table):page_table(page_table){
    // only reference++, need to add to page table
    segments[0]=other.segments[0];
    segments[0]->register_pageTable(page_table);

    // copy data segment, with page table added
    segments[1]=RefCountPtr<elf_ctl_segment>(
            new elf_ctl_segment(*other.segments[1],page_table)
            );
}

void Elf_Control::init_segments(){
    segments[0]=RefCountPtr<elf_ctl_segment>(new elf_ctl_segment(true));
    segments[1]=RefCountPtr<elf_ctl_segment>(new elf_ctl_segment(false));
}

void Elf_Control::bind_text_page(size_t target_v_addr,size_t source_mem_addr){
    segments[0]->add(page_table,target_v_addr,source_mem_addr);
}

void Elf_Control::bind_data_page(size_t target_v_addr,size_t source_mem_addr){
    segments[1]->add(page_table,target_v_addr,source_mem_addr);
}
