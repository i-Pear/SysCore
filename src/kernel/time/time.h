#ifndef OS_RISC_V_TIME_H
#define OS_RISC_V_TIME_H

#include "../posix/posix_structs.h"
#include "../../driver/types.h"

#define CLINT_MAX_CORES     (4095)

typedef struct _clint_msip
{
    uint32_t msip : 1;  /*!< Bit 0 is msip */
    uint32_t zero : 31; /*!< Bits [32:1] is 0 */
} __attribute__((packed, aligned(4))) clint_msip_t;

typedef uint64 clint_mtimecmp_t;

typedef uint64 clint_mtime_t;

typedef struct _clint
{
    /* 0x0000 to 0x3FF8, MSIP Registers */
    clint_msip_t msip[CLINT_MAX_CORES];
    /* Resverd space, do not use */
    uint32_t resv0;
    /* 0x4000 to 0xBFF0, Timer Compare Registers */
    clint_mtimecmp_t mtimecmp[CLINT_MAX_CORES];
    /* 0xBFF8, Time Register */
    clint_mtime_t mtime;
} __attribute__((packed, aligned(4))) clint_t;

uint64 r_time(void);
uint64 get_nsec(void);
uint64 get_usec(void);
uint64 get_msec(void);
uint64 get_sec(void);
// uint64 timer(void);
void init_rtc(void);
uint64 mktime(struct tm *timptr);
uint64 mktime_set(const uint64 year0, const uint64 mon0, const uint64 day, const uint64 hour, const uint64 min, const uint64 sec);
void current_timespec(struct timespec *ts);

typedef enum _clockid_t{
    CLOCK_REALTIME,
    CLOCK_MONOTONIC,
    CLOCK_PROCESS_CPUTIME_ID,
    CLOCK_THREAD_CPUTIME_ID
} clockid_t;

long clock_gettime(clockid_t which_clock, struct timespec *ts);
#endif //OS_RISC_V_TIME_H
