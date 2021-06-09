#ifndef OS_RISC_V_TEST_H
#define OS_RISC_V_TEST_H

#include "../lib/stl/string.h"

class Test{
private:
    int test_count;
    char* test_name;
public:
    Test(const char* name, int start_count = 0):test_count(start_count){
        test_name = (char *)k_malloc(strlen(name) + 1);
        strcpy(test_name, name);
    }

    virtual void start_one_test(){
        printf("#=> Start Test %s No.%d\n", test_name, test_count);
        test_count++;
    }

    virtual void end_one_test(){
        printf("<=# End Test %s No.%d\n", test_name, test_count);
    }
};

class TestString: public Test{
public:
    TestString(): Test("String", 0){}

    void test(){
        start_one_test();

        String s1 = "Hello World";
        String s2 = s1;
        assert(s1 == s2);
        String s3 = "Hello ";
        s3.append(String("World"));
        assert(s1 == s3);
        String s5 = (String("Hello") + String(" ")+ String("World"));
        printf("s5 %s\n", s5.c_str());
        assert(s5 == s1);
        String s4 = "Hello";
        s4 += " ";
        s4 += "World";
        assert(s4 == "Hello World");

        end_one_test();
    }
};

#endif //OS_RISC_V_TEST_H
