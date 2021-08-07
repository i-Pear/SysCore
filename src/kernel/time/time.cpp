#include "time.h"
#include "../../driver/rtc/rtc.h"
#include "../../driver/sysctl.h"

unsigned long mktime(const unsigned int year0,
                     const unsigned int mon0,
                     const unsigned int day,
                     const unsigned int hour,
                     const unsigned int min,
                     const unsigned int sec) {
    unsigned int mon = mon0, year = year0;

    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int) (mon -= 2)) {
        mon += 12; /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long) (year / 4 - year / 100 + year / 400 +
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

int leapYears[100];  // 1970~2070之间每年和1970年的闰年数量
uint32_t freq;

int setDateTime(int year, int month, int day, int hour, int minute, int second) {
    return rtc_timer_set(year, month, day, hour, minute, second);
}

int getDateTime(
        int *year, int *month, int *day, int *hour, int *minute, int *second) {
    return rtc_timer_get(year, month, day, hour, minute, second);
}

// 时间系统
void getTimeVal(TimeVal &timeVal) {
    struct tm *tm = rtc_timer_get_tm();

    int off = tm->tm_year - 70;  // tm->tm_year = year - 1900
    int leaps = leapYears[off - 1];
    uint64 days = leaps * 366 + (off - leaps) * 365 + tm->tm_yday;
    uint64 sec = days * 86400 + tm->tm_hour * 3600 + tm->tm_sec;
    uint32_t count = rtc_timer_get_clock_count_value();

    uint64 us = (count % freq) / (freq / 1000000);
    timeVal.sec = sec;
    timeVal.usec = us;
}

void initLeapYears() {
    int year = 0;
    year = 1970;
    leapYears[0] = 0;  // 1970不是闰年
    for (int i = 1; i < 100; i++) {
        year = i + 1970;
        if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) {
            leapYears[i] = leapYears[i - 1] + 1;
        } else {
            leapYears[i] = leapYears[i - 1];
        }
        // printf("leap years=%d\n", leapYears[i]);
    }
}

void init_rtc() {
    rtc_init();
    initLeapYears();
    freq = sysctl_clock_get_freq(SYSCTL_CLOCK_IN0);
    rtc_timer_set(2021, 6, 1, 23, 59, 59);
}

uint64 r_time (void) {
    uint64 res;
    asm volatile(
    "rdtime %0"
    :"=r"(res)
    );
    return res;
}

uint64 timer(void){
    uint64 usec = r_time() * 50 * 1000000UL / sysctl_clock_get_freq(SYSCTL_CLOCK_CPU);
    return usec;
}