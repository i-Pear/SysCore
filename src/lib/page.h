#ifndef __PAGE_H__
#define __PAGE_H__

#include <math.h>
#include <stl.h>

typedef enum {
    PAGE_ENTRY_FLAG_VALID = 1 << 0,      // 有效位
    PAGE_ENTRY_FLAG_READABLE = 1 << 1,   // 读
    PAGE_ENTRY_FLAG_WRITEABLE = 1 << 2,  // 写
    PAGE_ENTRY_FLAG_EXECUTABLE = 1 << 3, // 执行
    PAGE_ENTRY_FLAG_USER = 1 << 4,       // 用户位
    PAGE_ENTRY_FLAG_GLOBAL = 1 << 5,     // 全局位
    PAGE_ENTRY_FLAG_ACCESSED = 1 << 6,   // 已使用
    PAGE_ENTRY_FLAG_DIRTY = 1 << 7,       // 已修改
    PAGE_ENTRY_FLAGS_NEXT_ENTRY = 0xc1,
    PAGE_ENTRY_FLAGS_END_ENTRY = 0xdf,
} PAGE_ENTRY_FLAG;


void make_map(Map *mem_map, size_t *page_table_start, size_t vir_addr, size_t phy_addr);

void memory_map_init();

extern Map *memory_map;

#endif