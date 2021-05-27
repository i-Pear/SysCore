#include "times.h"
#include "sbi.h"

void get_timespec(TimeVal* timeVal){
    size_t time=read_time();
    size_t ms=time/1000;

    // seconds since 1970
    timeVal->sec=ms/1000;
    // micro-seconds
    timeVal->usec=ms%1000;
}
