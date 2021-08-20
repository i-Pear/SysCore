#ifndef OS_RISC_V_VECTOR_H
#define OS_RISC_V_VECTOR_H

#include "stdbool.h"
#include "stddef.h"
#include "stl.h"

template <typename T>
struct Vector{
    T* _data;
    int size;
    int capacity;

    Vector(){
        _data= nullptr;
        size=0;
        capacity=0;
    }

    Vector(const Vector<T>& other){
        size=capacity=other.size;
        _data=new T[size];
        for(int i=0;i<size;i++){
            _data[i]=other._data[i];
        }
    }

    T& operator [] (int n)const{
        assert(n<capacity);
        return _data[n];
    }

    int length()const{
        return size;
    }

    bool is_empty()const{
        return size==0;
    }

    void push_back(T data){
        if(size+1>capacity){
            capacity= max(capacity*2,1);
            T* new_data=new T[capacity];
            memcpy(new_data,_data, sizeof(T)*size);
            delete[] _data;
            _data=new_data;
        }
        _data[size++]=data;
    }

    void pop_back(){
        assert(size>0);
        size--;
    }

    void erase(const T& element){
        for(int i=0;i<size;i++){
            if(_data[i]==element){
                size--;
                for(;i<size;i++){
                    _data[i]=_data[i+1];
                }
                break;
            }
        }
    }

    void erase(int pos){
        size--;
        for(;pos<size;pos++){
            _data[pos]=_data[pos+1];
        }
    }

};


#endif //OS_RISC_V_VECTOR_H
