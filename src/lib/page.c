#include <page.h>

Map *memory_map;

size_t get_bits(size_t origin, size_t low, size_t len) {
    size_t mask = ppow(2, len) - 1;
    return (origin & (mask << low)) >> low;
}

void make_map(Map *mem_map, size_t *page_table_start, size_t vir_addr, size_t phy_addr) {
    Map_put(mem_map, vir_addr, phy_addr);
//    printf("Vir: 0x%x --> Phy: 0x%x\n", vir_addr, phy_addr);
    size_t *x = page_table_start + (get_bits(vir_addr, 30, 9) / sizeof(size_t));
    if (!(*x & PAGE_ENTRY_FLAG_VALID) || (*x & PAGE_ENTRY_FLAG_READABLE)) {
        size_t new_page = (size_t) alloc_page();
        *x = (new_page << 10) | PAGE_ENTRY_FLAGS_NEXT_ENTRY;
    }
    x = (size_t *) (*x >> 10) + (get_bits(vir_addr, 21, 9) / sizeof(size_t));
    if (!(*x & PAGE_ENTRY_FLAG_VALID) || (*x & PAGE_ENTRY_FLAG_READABLE)) {
        size_t new_page = (size_t) alloc_page();
        *x = (new_page << 10) | PAGE_ENTRY_FLAGS_NEXT_ENTRY;
    }
    x = (size_t *) (*x >> 10) + (get_bits(vir_addr, 12, 9) / sizeof(size_t));
    *x = (phy_addr >> 2) | PAGE_ENTRY_FLAGS_END_ENTRY;
    flush_tlb();
}

void memory_map_init() {
    Map_init(&memory_map);
    size_t kernel_start = __kernel_start;
    size_t heap_end = get_kernel_end() + __heap_page_num * __page_size;
    size_t pages = (heap_end - kernel_start) >> 12;
    for (size_t i = 0; i < pages; i++) {
        make_map(memory_map, (size_t *) get_boot_page_table(), kernel_start + (i << 12) + __kernel_vir_offset,
                 kernel_start + (i << 12));
    }
}