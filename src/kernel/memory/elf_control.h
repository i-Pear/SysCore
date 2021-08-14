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

/**
 * manages an elf_segment: contains many pages
 */
class elf_ctl_segment{
public:

    bool _is_text;
    Vector<elf_ctl_page> pages;

    explicit elf_ctl_segment(bool is_text);

    void add(size_t page_table,size_t target_v_addr,size_t source_mem_addr);

    void register_pageTable(size_t page_table);

    bool is_text() const;

    elf_ctl_segment(const elf_ctl_segment& other,size_t page_table);

    ~elf_ctl_segment();
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

    explicit Elf_Control(size_t page_table);

    Elf_Control(const Elf_Control& other,size_t page_table);

    void init_segments();

    void bind_text_page(size_t target_v_addr,size_t source_mem_addr);

    void bind_data_page(size_t target_v_addr,size_t source_mem_addr);
};

#endif //OS_RISC_V_ELF_CONTROL_H
