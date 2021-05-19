#ifndef __SDCARD_H
#define __SDCARD_H

#include "types.h"

void sdcard_init(void);

void sdcard_read_sector(uint8 *buf, int sectorno);

void sdcard_write_sector(uint8 *buf, int sectorno);

/**
 * 调用这个函数会导致U盘文件系统损坏，用了之后记得初始化sd卡
 */
void test_sdcard(void);

#endif
