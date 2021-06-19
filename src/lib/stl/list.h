#ifndef OS_RISC_V_LIST_H
#define OS_RISC_V_LIST_H

#include "stdbool.h"
#include "stddef.h"

template <typename T>
struct ListNode{
    T data;
    ListNode<T>* next;
    ListNode<T>* previous;

    ListNode<T>()= default;
};

template <typename T>
struct List{
    ListNode<T>* start;
    ListNode<T>* end;

    T& operator [] (int n){
        auto cnt=start;
        while (n--){
            assert(cnt!= nullptr);
            cnt=cnt->next;
        }
        assert(cnt!= nullptr);
        return cnt->data;
    }

    int length()const{
        auto cnt=start;
        int count=0;
        while (cnt!= nullptr){
            cnt=cnt->next;
            count++;
        }
        return count;
    }

    bool is_empty()const{
        return start==nullptr;
    }

    void push_back(T data){
        if(start==nullptr&&end==nullptr){
            // empty list
            auto* new_node=new ListNode<T>;
            new_node->data=data;
            new_node->next=nullptr;
            new_node->previous= nullptr;

            start=end=new_node;
        }else{
            auto* new_node=new ListNode<T>();
            new_node->data=data;
            new_node->next=nullptr;
            new_node->previous=end;
            // link
            end->next=new_node;
            end=new_node;
        }
    }

    void push_front(T data){
        if(start==nullptr&&end==nullptr){
            // empty list
            auto* new_node=new ListNode<T>;
            new_node->data=data;
            new_node->next=nullptr;
            new_node->previous= nullptr;

            start=end=new_node;
        }else{
            auto* new_node=new ListNode<T>;
            new_node->data=data;
            new_node->next=start;
            new_node->previous= nullptr;
            // link
            start->previous=new_node;
            start=new_node;
        }
    }

    void pop_front(){
        if(start==end){
            // has only one
            ListNode<T>* firstNode=start;
            start=end=nullptr;
            delete firstNode;
        }else{
            ListNode<T>* firstNode=start;
            start=firstNode->next;
            delete firstNode;
        }
    }

    void erase(ListNode<T>* pos){
        if(start==pos)start=pos->next;
        if(end==pos)end=pos->previous;
        if(pos->previous!=nullptr)pos->previous->next=pos->next;
        if(pos->next!=nullptr)pos->next->previous=pos->previous;
        delete pos;
    }

    List(){
        start=end=nullptr;
    }

    List(const List<T>& list){
        start=end=nullptr;
        ListNode<T>* cnt=list.start;
        while (cnt){
            push_back(cnt->data);
            cnt=cnt->next;
        }
    }

    List<T>& operator=(const List<T>& list){
        // TODO:free
        while (start!= nullptr){
            pop_front();
        }

        // copy
        start=end=nullptr;
        ListNode<T>* cnt=list.start;
        while (cnt){
            push_back(cnt->data);
            cnt=cnt->next;
        }
    }
};

#endif //OS_RISC_V_LIST_H
