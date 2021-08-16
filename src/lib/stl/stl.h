#ifndef __STL_H__
#define __STL_H__

//#include "../../kernel/memory/kernel_heap.h"
#include "../../kernel/stdio.h"
#include "../../kernel/sbi.h"

#define INT_MAX 2147483647
#define INT_MIN (-INT_MAX-1)

// #define debug_mode

#ifdef debug_mode
#define lty(a) printf(#a " = 0x%x\n",a)
#define mtl(x) printf("%s\n",x)
#else
#define lty(a) ;
#define mtl(x) ;
#endif

#define BENCHING

#define DDD() {printf("[DDD] %s:%d \n",__FILE__,__LINE__);}
#define panic(message) {printf("[panic] %s:%d  %s\n",__FILE__,__LINE__,message);shutdown();}

#ifndef BENCHING
#define assert(equal) if(!(equal)){printf("[assert] %s:%d error\n",__FILE__,__LINE__);shutdown();}
#else
#define assert(equal)
#endif

extern "C" void memcpy(void *to,const void *from, size_t size);

extern "C" void memset(void* p,char content,size_t size);

int strlen(const char *s);

void strcpy(char* to,const char* from);

int strcmp(const char *cs, const char *ct);

template<typename T>
inline T max(const T& a,const T& b) {
    return a > b ? a : b;
}

template<typename T>
inline T min(const T& a,const T& b) {
    return a < b ? a : b;
}

#endif