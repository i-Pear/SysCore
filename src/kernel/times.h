#ifndef OS_RISC_V_TIMES_H
#define OS_RISC_V_TIMES_H

#include "posix/posix_structs.h"
#include "stddef.h"

void get_timespec(TimeVal* timeVal);

void init_times();

extern size_t time_seconds;
extern size_t time_macro_seconds;

#endif //OS_RISC_V_TIMES_H
