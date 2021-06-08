#ifndef OS_RISC_V_STRING_H
#define OS_RISC_V_STRING_H

class String{
public:
    const static int __INIT_CAPACITY=10;
    int _capacity;
    int _length;
    char* data;

    String(const char* str){
        _length= strlen(str);
        _capacity=_length+1;
        data=new char[_capacity];
        memcpy(data,str,_length+1);
    }

    void operator=(const char* str){
        delete[] data;

        _length= strlen(str);
        _capacity=_length+1;
        data=new char[_capacity];
        memcpy(data,str,_length+1);
    }

    String(): _capacity(__INIT_CAPACITY), _length(0){
        data=new char[_capacity];
    }

    String(int length):_capacity(length+1),_length(0){
        data=new char[_capacity];
    }

    String(const String& b){
        _capacity=b._length;
        _length=b._length;
        data=new char[_length];
        memcpy(data,b.data,_length);
    }

    char* c_str()const{
        return data;
    }

    int length()const{
        return _length-1;
    }

    int size()const{
        return _length-1;
    }

    String operator+(const String& b)const{
        String result(_length+b._length);
        memcpy(result.data,data,_length);
        memcpy(result.data+_length,b.data,b._length);
        result.data[_length+b._length+1]='\0';
        return result;
    }

    void append(const String& b){
        if(_length+b._length+1>_capacity){
            int new_capacity=max(_capacity*2,_length+b._length+1);
            char* new_data=new char[new_capacity];
            memcpy(new_data,data,_length);
            memcpy(new_data+_length,b.data,b._length+1);
            delete[] data;
            data=new_data;
        }else{
            _length+=b._length;
            memcpy(data+_length,b.data,b._length+1);
        }
    }

    void operator+=(const String& b){
        append(b);
    }
};

#endif //OS_RISC_V_STRING_H
