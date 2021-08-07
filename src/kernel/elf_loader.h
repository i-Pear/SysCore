#ifndef OS_RISC_V_ELF_LOADER_H
#define OS_RISC_V_ELF_LOADER_H

#include <elf.h>
#include "stdio.h"
#include "../driver/fatfs/ff.h"
#include "memory/elf_control.h"

//#define DEBUG_ELF

void load_elf(FIL* elf_file,Elf_Control* elf_control,size_t* entry,Elf64_Off* e_phoff,int* phnum,Elf64_Phdr ** kernel_phdr);

#endif //OS_RISC_V_ELF_LOADER_H
