#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdio.h>

/*
    Interrupt Context
 */
typedef struct {
    size_t x[32];
    size_t sstatus;
    size_t sepc;
} Context;

Context *handle_interrupt(Context *context, size_t scause, size_t stval);

void set_next_timeout();

extern void close_interrupt();

extern void __restore(Context *context);

#endif