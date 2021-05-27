#ifndef OS_RISC_V_TIMES_H
#define OS_RISC_V_TIMES_H

#include "external_structs.h"
#include "stddef.h"

size_t __get_time();

void get_timespec(TimeVal* timeVal);

#endif //OS_RISC_V_TIMES_H
