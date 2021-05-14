#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdio.h>

/*
    Interrupt Context
 */
/**
 * Risc-V Registers
 *
 * x0/zero Hardwired Zero
 * x1/ra Return Address
 * x2/sp Stack Pointer
 * x3/gp Global Pointer
 * x4/tp Thread Pointer
 * x5/t0 Temporary
 * x6/t1 Temporary
 * x7/t2 Temporary
 * x8/s0/fp Saved Register, Frame Pointer
 * x9/s1 Saved Register
 * x10/a0 Function Argument, Return Value
 * x11/a1 Function Argument
 * x12/a2 Function Argument
 * x13/a3 Function Argument
 * x14/a4 Function Argument
 * x15/a5 Function Argument
 * x16/a6 Function Argument
 * x17/a7 Function Argument
 * x18/s2 Saved Register
 * x19/s3 Saved Register
 * x20/s4 Saved Register
 * x21/s5 Saved Register
 * x22/s6 Saved Register
 * x23/s7 Saved Register
 * x24/s8 Saved Register
 * x25/s9 Saved Register
 * x26/s10 Saved Register
 * x27/s11 Saved Register
 * x28/t3 Temporary
 * x29/t4 Temporary
 * x30/t5 Temporary
 * x31/t6 Temporary
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