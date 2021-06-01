#ifndef __STDIO_H__
#define __STDIO_H__

#include "sbi.h"
#include "stdarg.h"
#include "kernel_heap.h"

void putchar(char c);

void puts(char *str);

void printf(const char *format, ...);

void sprintf(char* dist, const char *format, ...);

size_t getchar_blocked();

#endif