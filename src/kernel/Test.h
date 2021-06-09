#ifndef OS_RISC_V_TEST_H
#define OS_RISC_V_TEST_H

#include "../lib/stl/string.h"

class TestString{
public:
    void test(){
        const char* test_name = "String";
        printf("#=> Start Test %s\n", test_name);

        String s1 = "Hello World";
        String s2 = s1;
        assert(s1 == s2);
        String s3 = "Hello ";
        s3.append(String("World"));
        assert(s1 == s3);
        String s5 = (String("Hello") + String(" ")+ String("World"));
        assert(s5 == s1);
        String s4 = "Hello";
        s4 += " ";
        s4 += "World";
        assert(s4 == "Hello World");

        printf("<=# End Test %s No.%d\n", test_name);
    }
};

#endif //OS_RISC_V_TEST_H
