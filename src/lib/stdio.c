#include <stdio.h>

size_t getchar() {
    size_t res = sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0);
    while (res == -1) {
        res = sbi_call(SBI_CONSOLE_GETCHAR, 0, 0, 0);
    }
    return res;
}

void putchar(char c) { sbi_call(SBI_CONSOLE_PUTCHAR, c, 0, 0); }

void puts(char *str) {
    char *p = str;
    while (*p != '\0') {
        putchar(*p);
        p++;
    }
    putchar('\n');
}

void __print_pos(size_t num, int base) {
    char sta[65];
    int cur = 0;
    if (!num) {
        sta[cur++] = '0';
    }
    while (num) {
        int tail = num % base;
        if (tail <= 9)
            sta[cur++] = '0' + tail;
        else
            sta[cur++] = 'a' + tail - 10;
        num /= base;
    }
    while (cur != 0)
        putchar(sta[--cur]);
}

void printf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    while (*format) {
        char ret = *format;
        if (ret == '%') {
            switch (*++format) {
                case 'c': {
                    char c = va_arg(arg, int);
                    putchar(c);
                    break;
                }
                case 'd': {
                    int d = va_arg(arg, int);
                    if (d == 0) {
                        putchar('0');
                    } else {
                        if (d < 0)
                            putchar('-'), d = -d;
                        __print_pos(d, 10);
                    }
                    break;
                }
                case 'x': {
                    size_t x = va_arg(arg, size_t);
                    __print_pos(x, 16);
                    break;
                }
                case 'b': {
                    size_t x = va_arg(arg, size_t);
                    __print_pos(x, 2);
                    break;
                }
                case 's': {
                    char *str = va_arg(arg, char *);
                    char *p = str;
                    while (*p != '\0') {
                        putchar(*p);
                        p++;
                    }
                    break;
                }
                default:
                    puts("!!! unsupport %");
            }
        } else {
            putchar(ret);
        }
        format++;
    }
}