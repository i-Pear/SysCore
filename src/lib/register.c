#include "register.h"

size_t register_read_sip() {
    size_t res;
    asm volatile(
    "csrr %0, sip"
    :"=r"(res)
    );
    return res;
}

size_t register_read_sstatus() {
    size_t res;
    asm volatile(
    "csrr %0, sstatus"
    :"=r"(res)
    );
    return res;
}

size_t register_read_sp() {
    size_t res;
    asm volatile(
    "mv %0, sp"
    :"=r"(res)
    );
    return res;
}