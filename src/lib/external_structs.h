#ifndef OS_RISC_V_EXTERNAL_STRUCTS_H
#define OS_RISC_V_EXTERNAL_STRUCTS_H

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
#define uint64 unsigned long long
typedef unsigned int uint;

struct ES_tms
{
    long tms_utime;   /* user cpu time */
    long tms_stime;   /* system cpu time */
    long tms_cutime;  /* user cpu time of children */
    long tms_cstime;  /* system cpu time of children */
};

struct ES_utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};

extern struct ES_utsname ES_uname;

typedef struct
{
    uint64 sec;  // 自 Unix 纪元起的秒数
    uint64 usec; // 微秒数
} TimeVal;

typedef struct
{
    uint64 dev;    // 文件所在磁盘驱动器号，不考虑
    uint64 ino;    // inode 文件所在 inode 编号
    uint32 mode;   // 文件类型
    uint32 nlink;  // 硬链接数量，初始为1
    uint64 pad[7]; // 无需考虑，为了兼容性设计
} Stat;

typedef unsigned int mode_t;
typedef long int off_t;

struct kstat {
    uint64 st_dev;
    uint64 st_ino;
    mode_t st_mode;
    uint32 st_nlink;
    uint32 st_uid;
    uint32 st_gid;
    uint64 st_rdev;
    unsigned long __pad;
    off_t st_size;
    uint32 st_blksize;
    int __pad2;
    uint64 st_blocks;
    long st_atime_sec;
    long st_atime_nsec;
    long st_mtime_sec;
    long st_mtime_nsec;
    long st_ctime_sec;
    long st_ctime_nsec;
    unsigned __unused[2];
};

typedef long long int64;

struct linux_dirent64 {
    uint64        d_ino;
    int64         d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char            d_name[];
};

#endif //OS_RISC_V_EXTERNAL_STRUCTS_H
