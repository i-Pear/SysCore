#include "time.h"
#include "../../driver/rtc/rtc.h"
#include "../../driver/sysctl.h"

uint64 r_time () {
    uint64 res;
    asm volatile(
    "rdtime %0"
    :"=r"(res)
    );
    return res;
}

uint64 timer(){
    uint64 usec = r_time() * 50 * 1000000UL / sysctl_clock_get_freq(SYSCTL_CLOCK_CPU);
    return usec;
}