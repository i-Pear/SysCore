#ifndef OS_RISC_V_TIME_H
#define OS_RISC_V_TIME_H

#include "../posix/posix_structs.h"
#include "../../driver/types.h"

void init_rtc();
uint64 r_time(void);
uint64 timer(void);

unsigned long mktime(const unsigned int year,
                     const unsigned int mon,
                     const unsigned int day,
                     const unsigned int hour,
                     const unsigned int min,
                     const unsigned int sec);

void CurrentTimeSpec(struct timespec *ts);



#endif //OS_RISC_V_TIME_H
