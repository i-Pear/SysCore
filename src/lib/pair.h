#ifndef OS_RISC_V_PAIR_H
#define OS_RISC_V_PAIR_H

template<typename Class1,typename Class2>
struct pair{
    Class1 first;
    Class2 second;

    pair()= default;

    pair(const Class1& first,const Class2& second):first(first),second(second){

    }

    bool operator<(const pair<Class1,Class2>& other)const{
        if(first!=other.first)return first<other.first;
        return second<other.second;
    }
};

template<typename Class1,typename Class2>
pair<Class1,Class2> make_pair(const Class1& first,const Class2& second){
    return pair<Class1,Class2>(first,second);
}

#endif //OS_RISC_V_PAIR_H
