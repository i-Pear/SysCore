#include "times.h"
#include "sbi.h"

size_t time_seconds=0;
size_t time_macro_seconds=0;

void init_times(){
    time_seconds=time_macro_seconds=0;
}

void get_timespec(TimeVal* timeVal){
    // size_t time=read_time();
    // size_t ms=time/1000;

    // seconds since 1970
    timeVal->sec=time_seconds;
    // micro-seconds
    timeVal->usec=time_macro_seconds;
}
