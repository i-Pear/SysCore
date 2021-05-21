#ifndef OS_RISC_V_SCHEDULER_H
#define OS_RISC_V_SCHEDULER_H

#include "stl.h"

void create_process(const char* elf_path);

extern size_t elf_exec_page_base_only_one;

#endif //OS_RISC_V_SCHEDULER_H
