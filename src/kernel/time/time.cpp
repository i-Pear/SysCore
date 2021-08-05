#include "time.h"
#include "../../driver/sysctl.h"

unsigned long mktime(const unsigned int year0,
                     const unsigned int mon0,
                     const unsigned int day,
                     const unsigned int hour,
                     const unsigned int min,
                     const unsigned int sec)
{
    unsigned int mon = mon0, year = year0;

    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int)(mon -= 2)) {
        mon += 12; /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long)(year / 4 - year / 100 + year / 400 +
                              367 * mon / 12 + day) +
              year * 365 - 719499) *
             24 +
             hour /* now have hours */
            ) * 60 +
            min /* now have minutes */
           ) * 60 +
           sec; /* finally seconds */

    // return ((((unsigned long)
    // 	  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
    // 	  year*365 - 719499
    //     )*24 + hour /* now have hours */
    //   )*60 + min /* now have minutes */
    // )*60 + sec; /* finally seconds */
}

void CurrentTimeSpec(struct timespec *ts) {
    int year, month, day, hour, minute, second;
    rtc_timer_get(&year, &month, &day, &hour, &minute, &second);
    long unix_time = mktime(year, month, day, hour, minute, second);
    ts->tv_sec = unix_time;
    uint32_t count = rtc_timer_get_clock_count_value();
    uint32_t freq = sysctl_clock_get_freq(SYSCTL_CLOCK_IN0);
    uint64 us = (count % freq) / (freq / 1000000);
    ts->tv_nsec = us * 1000;
}