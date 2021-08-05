#ifndef OS_RISC_V_TIME_H
#define OS_RISC_V_TIME_H

#include "../posix/posix_structs.h"

void init_rtc();

unsigned long mktime(const unsigned int year,
                     const unsigned int mon,
                     const unsigned int day,
                     const unsigned int hour,
                     const unsigned int min,
                     const unsigned int sec);

void CurrentTimeSpec(struct timespec *ts);

#endif //OS_RISC_V_TIME_H
