#ifndef OS_RISC_V_TIME_H
#define OS_RISC_V_TIME_H

#include "../posix/posix_structs.h"
#include "../../driver/types.h"

uint64 r_time(void);
uint64 timer(void);

#endif //OS_RISC_V_TIME_H
