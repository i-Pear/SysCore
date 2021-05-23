#include "self_test.h"

int test_cnt=0;
int test_total=11;
char* tests[20]={
        "fork",
        "clone",
        "write",
        "uname",
        "times",
        "getpid",
        "getppid",
        "read",
        "open",
        "close",
        "openat",
};

void start_self_tests(){
    create_process(get_next_test());
}

int has_next_test(){
    return test_cnt<test_total;
}

const char* get_next_test(){
    return tests[test_cnt++];
}

