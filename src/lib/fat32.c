#include "fat32.h"
#include "stdio.h"
#include "memory.h"
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

#define FAT_SECTOR_BUF_SIZE (0x4200)
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
    return root_addr + (cluster_num - 2) * dpb.sector_size_in_bytes * dpb.number_of_sectors_per_cluster;
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
            tree(calc_content(calc_cluster(file_start)), level + 1);
        }

        addr += sizeof file_start;
        file_start = *(struct Fat32Entry *) fat_read(addr, sizeof(struct Fat32Entry));
    }
}

int strcmp_u16(uint16_t *left, uint16_t *right) {
    while (*left || *right) {
        if (*left < *right) return -1;
        else if (*left > *right) return 1;
        left++;
        right++;
    }
    if (*left)return 1;
    if (*right)return -1;
    return 0;
}

/**
 * 查找起始项
 * @param file_name 文件路径
 * @param find 是否查找到， 找到为1,没找到为0
 * @return Fat32Entry
 */
struct Fat32Entry fat_find_file_entry(const char *file_name, int* find) {
    uint16_t file_entry_name[512];
    uint16_t search_file_name[512];
    size_t i = 0, offset = 0;
    while (file_name[i]) {
        search_file_name[i] = (uint16_t) file_name[i];
        i++;
    }
    search_file_name[i] = search_file_name[i + 1] = 0;
    i = 0;
    while (search_file_name[i++] != '/') {}
    offset = i;
    while (search_file_name[i] && search_file_name[i++] != '/') {}
    if (search_file_name[i - 1] == '/')
        search_file_name[i - 1] = 0;
    size_t addr = root_addr;
    struct Fat32Entry file = *(struct Fat32Entry *) fat_read(addr, sizeof(struct Fat32Entry));
    while (file.file_attributes) {
        file = analyze_fat32_entry(file, &addr, file_entry_name);
        if (calc_fat_by_entry(file) == 0 || file.file_attributes == 0) {
            addr += sizeof(struct Fat32Entry);
            file = *(struct Fat32Entry*) fat_read(addr, sizeof (struct Fat32Entry));
            continue;
        }
        if (strcmp_u16(file_entry_name, search_file_name + offset) == 0) {
            if (search_file_name[i] == 0){
                *find = 1;
                return file;
            }
            offset = i;
            while (search_file_name[i] && search_file_name[i++] != '/') {}
            if (search_file_name[i - 1] == '/')
                search_file_name[i - 1] = 0;
            file = *(struct Fat32Entry *) fat_read(calc_content(calc_cluster(file)), sizeof(struct Fat32Entry));
        }
        addr += sizeof(struct Fat32Entry);
        file = *(struct Fat32Entry*) fat_read(addr, sizeof(struct Fat32Entry));
    }
    *find = 0;
    struct Fat32Entry fake;
    return fake;
}

enum CLUSTER_FLAG get_cluster_flag(size_t fat) {
    switch (fat) {
        case 0x0: {
            return CLUSTER_UNUSED;
        }
        case 0x1: {
            return CLUSTER_RESERVED;
        }
        case 0x2 ... 0x0fffffef: {
            return CLUSTER_USED;
        }
        case 0x0ffffff0 ... 0x0ffffff6: {
            return CLUSTER_RESERVED_VALUE;
        }
        case 0x0fffffff7: {
            return CLUSTER_BAD;
        }
        case 0x0ffffff8 ... 0x0fffffff: {
            return CLUSTER_END;
        }
    }
    printf("Err CLUSTER!");
    return CLUSTER_BAD;
}

struct FatFile FatFile_init(struct Fat32Entry fat32Entry) {
    struct FatFile res = {
            .fat32Entry = fat32Entry,
            .cluster_num = 0,
            .fat = calc_fat_by_entry(fat32Entry),
            .content_start = calc_content(calc_cluster(fat32Entry)),
            .cluster_flag = 0};
    return res;
}

uint32_t calc_next_fat(uint32_t fat) {
    return *((uint32_t *) fat_read(fat_addr + fat * sizeof(uint32_t), sizeof(uint32_t)));
}

size_t read_a_sector(struct FatFile *file, char buf[]) {
    if (file->cluster_flag == CLUSTER_END) {
        return 0;
    }
    size_t len = dpb.sector_size_in_bytes * dpb.number_of_sectors_per_cluster;
    size_t file_size = file->fat32Entry.file_size;
    if (file_size - file->cluster_num * dpb.sector_size_in_bytes * dpb.number_of_sectors_per_cluster <= dpb.sector_size_in_bytes * dpb.number_of_sectors_per_cluster) {
        len = file_size - file->cluster_num * dpb.sector_size_in_bytes * dpb.number_of_sectors_per_cluster;
    }
    void* res = fat_read(file->content_start, len);
    memcpy(buf, res, len);
    enum CLUSTER_FLAG flag = get_cluster_flag(file->fat);
    file->cluster_flag = flag;
    if (file->cluster_flag != CLUSTER_END) {
        file->content_start = calc_content(file->fat);
        file->fat = calc_next_fat(file->fat);
        file->cluster_num++;
    }
//    printf("len = 0x%x\n", len);
    return len;
}

void fat32_init() {
    fat_read(0, 512);
    dpb = *(struct DPB *) sector_buf;
    root_addr = (dpb.reserved_sectors + dpb.number_of_fats * dpb.number_sectors_in_the_one_fat) *
                dpb.sector_size_in_bytes;
    fat_addr = dpb.reserved_sectors * dpb.sector_size_in_bytes;
}

/**
 * 读取通过表项读取fat文件
 * @param fat32Entry fat32表项
 * @param buf 缓冲区
 * @param len 长度
 * @param offset 偏移
 * @return
 */
int fat_read_file(struct Fat32Entry fat32Entry, char buffer[]){
    struct FatFile file = FatFile_init(fat32Entry);
    size_t len, cur = 0;
    char buf[dpb.sector_size_in_bytes * dpb.number_of_sectors_per_cluster];
    do {
        len = read_a_sector(&file, buf);
        for(int i = 0;i < len; i++){
            buffer[cur + i] = buf[i];
        }
        cur += len;
    } while (len);
    return (int)cur;
}

uint32_t fat_calculate_file_size(struct Fat32Entry fat32Entry){
    return fat32Entry.file_size;
}

void test_fat32() {
    fat32_init();
    tree(root_addr, 0);
    int find = 0;
    struct Fat32Entry fileEntry = fat_find_file_entry("/lty", &find);
    if(find){
        printf("find!\n");
        struct FatFile file = FatFile_init(fileEntry);
        size_t len;
        char buf[512];
        do {
            len = read_a_sector(&file, buf);
            printf("Receive %d bytes.\n", len);
            for (int i = 0; i < len; i++) {
                printf("%x ", buf[i]);
                if (i != 0 && (i + 1) % 16 == 0)printf("\n");
            }
            printf("\n");
        } while (len);
    }else{
        printf("now find!\n");
    }
}