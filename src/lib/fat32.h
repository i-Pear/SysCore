#ifndef __FAT_32_H__
#define __FAT_32_H__

#include "stddef.h"

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct Fat32Entry {
    char short_file_name[8];
    uint8_t short_file_extension[3];
    uint8_t file_attributes;
    uint8_t extended_attributes;
    uint8_t create_time_unit_10ms;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint8_t high_two_bytes_of_the_first_cluster[2];
    uint16_t delete_time;
    uint16_t delete_date;
    uint8_t low_two_bytes_of_the_first_cluster[2];
    uint32_t file_size;
}__attribute__((packed));

void tree_all();

void test_fat32();
/**
 * 初始化fat文件系统
 */
void fat32_init();
/**
 * 查找起始项
 * @param file_name 文件路径
 * @param find 是否查找到， 找到为1,没找到为0
 * @return Fat32Entry
 */
struct Fat32Entry fat_find_file_entry(const char *file_name, int* find);
/**
 * 读取通过表项读取fat文件
 * @param fat32Entry fat32表项
 * @param buf 缓冲区
 * @param len 长度
 * @param offset 偏移
 * @return
 */
int fat_read_file(struct Fat32Entry fat32Entry, char buffer[]);
/**
 * 计算fat32文件大小
 * @param fat32Entry
 * @return 文件大小
 */
uint32_t fat_calculate_file_size(struct Fat32Entry fat32Entry);

#endif