#ifndef OS_RISC_V_SELF_TEST_H
#define OS_RISC_V_SELF_TEST_H

#include "stddef.h"

extern int test_cnt;
extern int test_total;
extern char* tests[];

void start_self_tests();

int has_next_test();

const char* get_next_test();

#endif //OS_RISC_V_SELF_TEST_H
