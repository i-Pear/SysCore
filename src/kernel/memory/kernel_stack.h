#ifndef OS_RISC_V_KERNEL_STACK_H
#define OS_RISC_V_KERNEL_STACK_H

#include "../stddef.h"

#define __kernel_stack_size 4096

extern size_t get_kernel_stack_end();

extern size_t get_kernel_stack_base();

extern char __kernel_stack_base[];

#endif //OS_RISC_V_KERNEL_STACK_H
