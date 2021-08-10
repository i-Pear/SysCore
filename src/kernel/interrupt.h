#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include "stdio.h"

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
struct Context{
    size_t zero=0;  // Hard-wired zero
    size_t ra=0;    // Return address
    size_t sp=0;    // Stack pointer
    size_t gp=0;    // Global pointer
    size_t tp=0;    // Thread pointer
    size_t t0=0;    // Temporary
    size_t t1=0;    // Temporary
    size_t t2=0;    // Temporary
    size_t s0=0;    // Saved register/frame pointer
    size_t s1=0;    // Saved register
    size_t a0=0;    // Function argument/return value
    size_t a1=0;    // Function argument/return value
    size_t a2=0;    // Function argument
    size_t a3=0;    // Function argument
    size_t a4=0;    // Function argument
    size_t a5=0;    // Function argument
    size_t a6=0;    // Function argument
    size_t a7=0;    // Function argument
    size_t s2=0;    // Saved register
    size_t s3=0;    // Saved register
    size_t s4=0;    // Saved register
    size_t s5=0;    // Saved register
    size_t s6=0;    // Saved register
    size_t s7=0;    // Saved register
    size_t s8=0;    // Saved register
    size_t s9=0;    // Saved register
    size_t s10=0;   // Saved register
    size_t s11=0;   // Saved register
    size_t t3=0;    // Temporary
    size_t t4=0;    // Temporary
    size_t t5=0;    // Temporary
    size_t t6=0;    // Temporary
    size_t sstatus=0;
    size_t sepc=0;
    size_t stval=0;
    size_t scause=0;
    size_t satp=0;
};

struct KernelContext{
    size_t kernel_satp;
    size_t kernel_sp;
    size_t kernel_handle_interrupt;
    size_t kernel_restore;
};

extern KernelContext kernelContext;

Context *handle_interrupt(Context *context, size_t scause, size_t stval);

void set_next_timeout();

extern void close_interrupt();

extern "C" void __restore();

#endif