#include "interface.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "spi.h"
#include "sysctl.h"

void *memmove(void *dst, const void *src, uint n) {
    const char *s;
    char *d;

    s = src;
    d = dst;
    if (s < d && s + n > d) {
        s += n;
        d += n;
        while (n-- > 0)
            *--d = *--s;
    } else
        while (n-- > 0)
            *d++ = *s++;

    return dst;
}

void uniform_init(){
    fpioa_pin_init();
}
