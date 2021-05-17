#ifndef OS_RISC_V_ELF_LOADER_H
#define OS_RISC_V_ELF_LOADER_H

#include <elf.h>
#include "stdio.h"
#include "memory.h"

void* elf_read(void** source, int size);

void* load_elf(const char* elf_data,int size);

#endif //OS_RISC_V_ELF_LOADER_H
