#ifndef OS_RISC_V_STRING_H
#define OS_RISC_V_STRING_H

#include "stl.h"

class String{
public:
    const static int __INIT_CAPACITY=10;
    int _capacity;
    int _length;
    char* data;

    String(const char* str);

    String& operator=(const char* str);

    String();

    String(int length);

    String(const String& b);

    char* c_str()const;

    int length()const;

    int size()const;

    String operator+(const String& b)const;

    void append(const String& b);

    void operator+=(const String& b);

    bool operator==(const String& b) const;

    bool operator<(const String& b)const;
};

String to_string(long long n);

#endif //OS_RISC_V_STRING_H
