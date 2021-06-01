#include "sbi.h"

void shutdown() { sbi_call(SBI_SHUTDOWN, 0, 0, 0); }

void set_timer(size_t time) { sbi_call(SBI_SET_TIMER, time, 0, 0); }

