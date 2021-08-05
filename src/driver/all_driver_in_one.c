#include "fpioa.c"
#include "gpiohs.c"
#include "interface.c"
#include "sdcard.c"
#include "spi.c"
#include "sysctl.c"
#include "utils.c"

#include "rtc/rtc.c"

// fatfs
#include "fatfs/ff.c"
#include "fatfs/diskio.c"
#include "fatfs/ffunicode.c"
#include "fatfs/ffsystem.c"


// img
#ifdef QEMU
#include "img.h"
#endif