#ifndef __STDIO_H__
#define __STDIO_H__

#include "sbi.h"
#include "stdarg.h"
//#include "memory/kernel_heap.h"

void putchar(char c);

void puts(char *str);

extern "C" void printf(const char *format, ...);

//#define LOG(x, ...) printf(x, __VA_ARGS__)
#define LOG(x, ...)


void sprintf(char* dist, const char *format, ...);

size_t getchar_blocked();

#endif