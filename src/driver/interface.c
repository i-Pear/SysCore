#include "interface.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "spi.h"
#include "sysctl.h"
#include "fatfs/ff.h"

void *memmove(void *dst,void *src, uint n) {
    char *s;
    char *d;

    s = (char *)src;
    d = (char *)dst;
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

void bsp_init(){
    fpioa_pin_init();
}
FATFS fs;
void fatfs_init(){
    FRESULT res_sd;
    res_sd = f_mount(&fs, "", 1);
    if (res_sd != FR_OK) {
        printf("fat init failed\n");
    }
}
void open_memory_lock(){
    sysctl_pll_enable(SYSCTL_PLL1);
    sysctl_clock_enable(SYSCTL_CLOCK_PLL1);
}
#ifdef QEMU
extern unsigned char fs_img[];
#endif
void driver_init(){
#ifndef QEMU
    bsp_init();
    open_memory_lock();
#endif
    fatfs_init();
}