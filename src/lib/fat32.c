#include "fat32.h"
#include "stdio.h"
#include "../driver/sdcard.h"

#define FILE_ATTR_SUB_DIRECTORY 0x10
#define FILE_ATTR_FILENAME 0x0f
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

struct DPB {
    uint8_t jmp_code[3];
    uint8_t oem_name[8];
    uint16_t sector_size_in_bytes;
    uint8_t number_of_sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t number_of_fats;
    uint16_t NA_0x11;
    uint16_t total_number_of_sectors_on_the_disk;
    uint8_t media_descriptor;
    uint16_t NA_0x16;
    uint16_t number_of_sectors_per_track;
    uint16_t number_of_heads;
    uint32_t number_of_hidden_sectors;
    uint32_t total_number_of_sectors;
    uint32_t number_sectors_in_the_one_fat;
    uint16_t flags_for_fat;
    uint16_t file_system_version_number;
    uint32_t cluster_number_for_the_root_directory;
    uint16_t sector_number_for_the_fs_info_structure;
    uint16_t sector_number_for_a_backup_copy_of_the_boot_sector;
    uint8_t reserved_for_future_expansion[12];
    uint8_t drive_number;
    uint8_t current_head;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system_type[8];
}__attribute__((packed)) dpb;

struct LongFileName {
    uint8_t entry_order_number;
    uint16_t file_name_1_5[5];
    uint8_t file_attribute;
    uint8_t subcomponent;
    uint8_t checksum_of_short_file_name;
    uint16_t file_name_6_11[6];
    uint16_t zero;
    uint16_t file_name_12_13[2];
}__attribute__((packed));

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

struct FatFile {
    struct Fat32Entry fat32Entry;
    size_t cluster_num;
    uint32_t fat;
    size_t content_start;
    size_t cluster_flag;
};

enum CLUSTER_FLAG {
    CLUSTER_UNUSED, CLUSTER_RESERVED, CLUSTER_USED, CLUSTER_RESERVED_VALUE, CLUSTER_BAD, CLUSTER_END
};



uint8 first_sector[512];
size_t root_addr;
size_t fat_addr;


size_t fat_get_sector_by_address(size_t addr) {
    return (addr | 0x1ff) ^ 0x1ff;
}

#define FAT_SECTOR_BUF_SIZE (512 * 2)
uint8 sector_buf[FAT_SECTOR_BUF_SIZE];

uint8 *fat_read(size_t addr, size_t len) {
    static size_t old_sector_addr = -1, old_count = -1;
    size_t sector_addr = fat_get_sector_by_address(addr);
    size_t count = (fat_get_sector_by_address(addr + len) - sector_addr) / 512 + 1;
    if (count * 512 > FAT_SECTOR_BUF_SIZE) {
        printf("[FAT panic!] FAT_SECTOR_BUF_SIZE should bigger than count * 512, FAT_SECTOR_BUF_SIZE = 0x%x, count * 512 = 0x%x",
               FAT_SECTOR_BUF_SIZE, count * 512);
    }
    if (sector_addr == old_sector_addr && old_count == count) {
//        printf("[FAT] Cached!, addr = 0x%x\n", sector_addr);
        return sector_buf + addr - sector_addr;
    }
//    printf("[FAT] read %d sectors, addr = 0x%x\n", count, sector_addr);
    int sectorno = (int) (sector_addr >> 9);
    for (size_t i = 0; i < count; i++) {
        sdcard_read_sector(sector_buf + i * 512, sectorno + i);
    }
    old_sector_addr = sector_addr;
    old_count = count;
    return sector_buf + addr - sector_addr;
}

void print_buf() {
    for (int i = 0; i < 512; i++) {
        if (i % 16 == 0)printf("\n");
        printf("%x ", sector_buf[i]);
    }
}

struct Fat32Entry analyze_fat32_entry(struct Fat32Entry file_start, size_t *file_start_addr, uint16_t *file_name) {
    uint32_t file_name_d = 0;
    while (file_start.file_attributes != 0) {
        if (file_start.file_attributes == FILE_ATTR_FILENAME) {
            struct LongFileName *fileName = (struct LongFileName *) &file_start;
            for (int i = 0; i < 5; i++)
                file_name[file_name_d++] = fileName->file_name_1_5[i];
            for (int i = 0; i < 6; i++)
                file_name[file_name_d++] = fileName->file_name_6_11[i];
            for (int i = 0; i < 2; i++)
                file_name[file_name_d++] = fileName->file_name_12_13[i];
        } else {
            if (file_name_d == 0) {
                for (int i = 0; i < 8; i++) {
                    file_name[file_name_d++] = file_start.short_file_name[i];
                }
            }
            break;
        }
        *file_start_addr += sizeof(struct Fat32Entry);
        file_start = *(struct Fat32Entry *) fat_read(*file_start_addr, sizeof(struct Fat32Entry));
    }
    file_name[file_name_d++] = '\0';
    return file_start;
}

void p_uint16(uint16_t *str) {
    while (*str != '\0') {
        putchar(*str);
//        printf("%x ", *str);
        str++;
    }
}

uint32_t calc_cluster(struct Fat32Entry fat32Entry) {
    uint32_t res = 0;
    res += fat32Entry.low_two_bytes_of_the_first_cluster[0];
    res += fat32Entry.low_two_bytes_of_the_first_cluster[1] * 256;
    res += fat32Entry.high_two_bytes_of_the_first_cluster[0] * 256 * 256;
    res += fat32Entry.high_two_bytes_of_the_first_cluster[1] * 256 * 256 * 256;
    return res;
}

uint32_t calc_fat_by_entry(struct Fat32Entry file) {
    return *((uint32_t *) fat_read(fat_addr + calc_cluster(file) * sizeof(uint32_t), sizeof(uint32_t)));
}

int strcmp_s(char *left, char *right, size_t len) {
    for (size_t i = 0; i < len; i++, left++, right++) {
        if (*left < *right) return -1;
        else if (*left > *right) return 1;
    }
    return 0;
}

size_t calc_content(size_t cluster_num) {
    return root_addr + (cluster_num - 2) * dpb.sector_size_in_bytes;
}

void tree(size_t addr, int level) {
    struct Fat32Entry file_start = *(struct Fat32Entry *) fat_read(addr, sizeof(struct Fat32Entry));
    uint16_t file_name[512];
    while (file_start.file_attributes) {
        file_start = analyze_fat32_entry(file_start, &addr, file_name);
        if (calc_fat_by_entry(file_start) == 0) {
            addr += sizeof file_start;
            file_start = *(struct Fat32Entry *) fat_read(addr, sizeof(struct Fat32Entry));
            continue;
        }
        for (int i = 0; i < level; i++)
            putchar(' ');
        p_uint16(file_name);
        if (file_start.file_attributes == FILE_ATTR_SUB_DIRECTORY)
            putchar('/');
        putchar('\n');
        if (file_start.file_attributes == FILE_ATTR_SUB_DIRECTORY
            && strcmp_s(file_start.short_file_name, ".       ", 8) != 0
            && strcmp_s(file_start.short_file_name, "..      ", 8) != 0){
            //TODO 遍历子文件夹失败
            tree(calc_content(calc_cluster(file_start)), level + 1);
        }

        addr += sizeof file_start;
        file_start = *(struct Fat32Entry *) fat_read(addr, sizeof(struct Fat32Entry));
    }
}

void fat32_init() {
    fat_read(0, 512);
    dpb = *(struct DPB *) sector_buf;
    root_addr = (dpb.reserved_sectors + dpb.number_of_fats * dpb.number_sectors_in_the_one_fat) *
                dpb.sector_size_in_bytes;
    fat_addr = dpb.reserved_sectors * dpb.sector_size_in_bytes;
    printf("[FAT] root addr: 0x%x\n", root_addr);
    printf("[FAT] fat addr: 0x%x\n", fat_addr);
}

void test_fat32() {
    fat32_init();
    tree(root_addr, 0);
}