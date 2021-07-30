#ifndef OS_RISC_V_MAP_H
#define OS_RISC_V_MAP_H

#include "list.h"
#include "stl.h"
#include "pair.h"

template<typename Key,typename Value>
struct Map{
    List<pair<Key,Value>> data;

    Map(){

    }

    Map(const Map<Key,Value>& other){
        if(other.is_empty())return;
        auto cnt=other.data.start;
        while (cnt!=nullptr){
            data.push_back(cnt->data);
            cnt=cnt->next;
        }
    }

    Map<Key,Value>& operator=(const Map<Key,Value>& other){
        data=other.data;
    }

    void put(Key key,Value value){
        auto cnt=data.start;
        while (cnt!=nullptr){
            if(cnt->data.first==key){
                cnt->data.second=value;
                return;
            }
            cnt=cnt->next;
        }
        // push front
        data.push_front({key,value});
    }

    void erase(Key key){
        // TODO: erase in map
    }

    bool is_empty()const{
        return data.is_empty();
    }

    bool exists(Key key)const{
        auto cnt=data.start;
        while (cnt!=nullptr){
            if(cnt->data.first==key){
                return true;
            }
            cnt=cnt->next;
        }
        return false;
    }

    Value& get(Key key){
        auto cnt=data.start;
        while (cnt!=nullptr){
            if(cnt->data.first==key){
                return cnt->data.second;
            }
            cnt=cnt->next;
        }
        printf("[panic] %s:%d  %s\n",__FILE__,__LINE__,"");shutdown();
    }
};

#endif //OS_RISC_V_MAP_H
