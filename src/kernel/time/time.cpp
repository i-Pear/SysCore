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

// #define CLINT_BASE_ADDR     (0x02000000U)

// volatile clint_t *const clint = (volatile clint_t *)CLINT_BASE_ADDR;

uint64 get_nsec(void){
    return r_time() * 1600000UL / sysctl_clock_get_freq(SYSCTL_CLOCK_CPU) * 50 * 625;
}

uint64 get_usec(void){
    return get_nsec() / 1000;
}

uint64 get_msec(void){
    return get_nsec() / 1000000;
}

uint64 get_sec(void){
    return get_nsec() / 1000000000;
}

uint64 timer(){
    // uint64 x = r_time();
    // uint64 usec = x / 10;
//    uint64 usec = x * 50 * 1000000UL / 406250;
    uint64 usec = get_usec();
    return usec;
}

void init_rtc(){
    rtc_init();
    rtc_timer_set(1970, 1, 1, 0, 0, 0); // UTC 1970-1-1 0:0:0 +0
}

uint64 mktime(struct tm *timeptr){
    return mktime_set(timeptr->tm_year, timeptr->tm_mon, timeptr->tm_mday, timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
}

uint64 mktime_set(const uint64 year, const uint64 month, const uint64 mday, const uint64 hour, const uint64 minute, const uint64 second) {
    uint64 mon = month, year_temp = year;

    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int) (mon -= 2)) {
        mon += 12; /* Puts Feb last since it has leap day */
        year_temp -= 1;
    }

    return ((((uint64) (year_temp / 4 - year_temp / 100 + year_temp / 400 + 367 * mon / 12 + mday) + year_temp * 365 - 719499
            ) * 24 +
             hour /* now have hours */
            ) * 60 +
             minute /* now have minutes */
            ) * 60 +
             second; /* finally seconds */
}

void current_timespec(struct timespec *ts) {
    int year, month, day, hour, minute, second;
    // printf("1\n");
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    // printf("%d %d %d %d %d %d\t",year,month,day,hour,minute,second);
    long unix_time = mktime_set(year, month, day, hour, minute, second);
    ts->tv_sec = unix_time;
    uint64 nsec = get_nsec() ;
    ts->tv_nsec = nsec % 1000000000;
}

long sys_clock_gettime(clockid_t which_clock, struct timespec *ts){
    uint64 nsec;
    switch(which_clock){
        case CLOCK_REALTIME:
            current_timespec(ts);
            break;
        case CLOCK_MONOTONIC:
            nsec = get_nsec();
            ts->tv_sec = nsec / 1000000000;
            ts->tv_nsec = nsec % 1000000000;
            break;
        // case CLOCK_PROCESS_CPUTIME_ID:
        //     break;
        // case CLOCK_THREAD_CPUTIME_ID:
        //     break;
        default:
            ts->tv_sec=0;
            ts->tv_nsec=0;
            break;
    }
}