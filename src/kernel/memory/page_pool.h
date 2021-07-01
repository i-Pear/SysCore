#ifndef OS_RISC_V_PAGE_POOL_H
#define OS_RISC_V_PAGE_POOL_H

#include "../lib/stl/list.h"
#include "memory.h"

enum MMAP_TYPE{UNKNOWN,ELF,HEAP,FILE};

struct Mmap_unit{
    MMAP_TYPE mmap_Type;
    size_t page_base;
    size_t virtual_addr;
    String* source;
    Mmap_unit(){
        mmap_Type=UNKNOWN;
        page_base= 0;
        virtual_addr= 0;
        source= nullptr;
    }
    // for file
    Mmap_unit(size_t page_base,size_t virtual_addr,const String& s){
        mmap_Type=FILE;
        this->page_base=page_base;
        this->virtual_addr=virtual_addr;
        source=new String(s);
    }
    // for heap
    Mmap_unit(size_t page_base,size_t virtual_addr,bool acquireSave){
        mmap_Type=acquireSave?HEAP:ELF;
        this->page_base=page_base;
        this->virtual_addr=virtual_addr;
        source= nullptr;
    }
    ~Mmap_unit(){
        delete source;
    }
};

class PagePool{
private:
    static PagePool* instance;
    // replace to vector
    List<Mmap_unit> mappings;

public:
    static PagePool& getInstance(){
        if(instance== nullptr){
            instance=new PagePool;
        }
        return *instance;
    }

    size_t get_available_page(){
        size_t addr=::alloc_page(PG_SIZE);
        return addr;
    }

    size_t alloc_elf_page(size_t page_base,size_t virtual_addr){
        size_t addr=get_available_page();
        mappings.push_back({page_base,virtual_addr,false});
        return addr;
    }

    // need file path to write back
    size_t alloc_file_page(size_t page_base,size_t virtual_addr,const String& filePath){
        size_t addr=get_available_page();
        // enable writeBack
        mappings.push_back({page_base,virtual_addr,filePath});
        return addr;
    }

    size_t alloc_heap_page(size_t page_base,size_t virtual_addr){
        size_t addr=get_available_page();
        mappings.push_back({page_base,virtual_addr, true});
        return addr;
    }

    void dealloc_page(int id){

    }
};

PagePool* PagePool::instance= nullptr;

#endif //OS_RISC_V_PAGE_POOL_H
