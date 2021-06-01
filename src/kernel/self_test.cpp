#include "self_test.h"
#include "stdio.h"

int test_cnt;
int test_total;
const char* tests[50];

void add_test(const char* name){
    tests[test_total++]=name;
}

void init_self_tests(){
    test_cnt=0;
    test_total=0;
}

int has_next_test(){
    return test_cnt<test_total;
}

const char* get_next_test(){
    printf("Running %s\n",tests[test_cnt]);
    return tests[test_cnt++];
}
