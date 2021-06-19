#include "string.h"

String::String(const char* str){
    _length= strlen(str);
    _capacity=_length+1;
    data=new char[_capacity];
    memcpy(data,str,_length+1);
}

String& String::operator=(const char* str){
    delete[] data;

    _length= strlen(str);
    _capacity=_length+1;
    data=new char[_capacity];
    memcpy(data,str,_length+1);

    return *this;
}

String::String(): _capacity(__INIT_CAPACITY), _length(0){
    data=new char[_capacity];
    memset(data,0,_capacity);
}

String::String(int length):_capacity(length+1),_length(length){
    data=new char[_capacity];
    memset(data,0,_capacity);
}

String::String(const String& b){
    _capacity=b._length+1;
    _length=b._length;
    data=new char[_capacity];
    memcpy(data,b.data,_capacity);
}

char* String::c_str()const{
    return data;
}

int String::length()const{
    return _length;
}

int String::size()const{
    return _length;
}

String String::operator+(const String& b)const{
    String result(_length+b._length);
    memcpy(result.data,data,_length);
    memcpy(result.data+_length,b.data,b._length);
    result.data[_length+b._length+1]='\0';
    return result;
}

void String::append(const String& b){
    if(_length+b._length+1>_capacity){
        int new_capacity=max(_capacity*2,_length+b._length+1);
        char* new_data=new char[new_capacity];
        memcpy(new_data,data,_length);
        memcpy(new_data+_length,b.data,b._length+1);
        delete[] data;
        data=new_data;
        _capacity=new_capacity;
        _length+=b._length;
    }else{
        memcpy(data+_length,b.data,b._length+1);
        _length+=b._length;
    }
}

void String::operator+=(const String& b){
    append(b);
}

bool String::operator==(const String& b) const{
    if(_length!=b._length)return false;
    for(int i=0;i<_length;i++){
        if(data[i]!=b.data[i])return false;
    }
    return true;
}

bool String::operator<(const String& b)const{
    return strcmp(data,b.data)<0;
}

String to_string(long long n){
    if(n==0){
        return String("0");
    }else if(n>0){
        char buf[30];
        int length=0;
        while (n){
            buf[30-1-(length++)]=n%10+'0';
            n/=10;
        }
        return String(&buf[30-length]);
    }else{
        n=-n;
        char buf[30];
        int length=0;
        while (n){
            buf[30-1-(length++)]=n%10+'0';
            n/=10;
        }
        buf[30-1-(length++)]='-';
        return String(&buf[30-length]);
    }
}

String to_string(unsigned long long n){
    if(n==0){
        return String("0");
    }else{
        char buf[30];
        int length=0;
        while (n){
            buf[30-1-(length++)]=n%10+'0';
            n/=10;
        }
        return String(&buf[30-length]);
    }
}

ostream& operator <<(ostream& os,const String& s){
    for(int i=0;i<s._length;i++){
        putchar(s.data[i]);
    }
    return os;
}

ostream& operator <<(ostream& os,long long n){
    os<< to_string(n);
    return os;
}

ostream& operator <<(ostream& os,unsigned long long n){
    os<< to_string(n);
    return os;
}

ostream cout;
