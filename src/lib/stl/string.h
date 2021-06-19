#ifndef OS_RISC_V_STRING_H
#define OS_RISC_V_STRING_H

#include "stl.h"

class String{
public:
    const static int __INIT_CAPACITY=10;
    int _capacity; // current vector capacity
    int _length; // length without '0' in the end
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

    char& operator[](const int x) const{
        return data[x];
    }
};

String to_string(long long n);
String to_string(unsigned long long n);

class ostream{
public:
    friend ostream& operator <<(ostream& os,const String& s);

    friend ostream& operator <<(ostream& os,long long n);

    friend ostream& operator <<(ostream& os,unsigned long long n);
};

extern ostream cout;

#endif //OS_RISC_V_STRING_H
