#ifndef OS_RISC_V_POSIX_STRUCTS_H
#define OS_RISC_V_POSIX_STRUCTS_H

#include "../stddef.h"

typedef char int8;
typedef unsigned char uint8;
typedef unsigned char uint8_t;
typedef short int16;
typedef short int16_t;
typedef unsigned short uint16;
typedef unsigned short uint16_t;
typedef int int32;
typedef unsigned int uint32;
typedef unsigned int uint32_t;
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
    unsigned long long sec;  // 自 Unix 纪元起的秒数
    unsigned long long usec; // 微秒数
} TimeVal;

struct timespec {
    long   tv_sec;        /* seconds */
    long   tv_nsec;       /* nanoseconds [0 .. 999999999] */
};


typedef struct
{
    unsigned long long dev;    // 文件所在磁盘驱动器号，不考虑
    unsigned long long ino;    // inode 文件所在 inode 编号
    uint32 mode;   // 文件类型
    uint32 nlink;  // 硬链接数量，初始为1
    unsigned long long pad[7]; // 无需考虑，为了兼容性设计
} Stat;

typedef unsigned int mode_t;
typedef long int off_t;

struct kstat {
    unsigned long long st_dev;
    unsigned long long st_ino;
    mode_t st_mode;
    uint32 st_nlink;
    uint32 st_uid;
    uint32 st_gid;
    unsigned long long st_rdev;
    unsigned long __pad;
    off_t st_size;
    uint32 st_blksize;
    int __pad2;
    unsigned long long st_blocks;
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
    unsigned long long        d_ino;
    int64         d_off;
    unsigned short  d_reclen;
    unsigned char   d_type;
    char            d_name[];
};

struct iovec {
    void  *iov_base;    /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};

struct sysinfo {
    long uptime;             /* Seconds since boot */
    unsigned long loads[3];  /* 1, 5, and 15 minute load averages */
    unsigned long totalram;  /* Total usable main memory size */
    unsigned long freeram;   /* Available memory size */
    unsigned long sharedram; /* Amount of shared memory */
    unsigned long bufferram; /* Memory used by buffers */
    unsigned long totalswap; /* Total swap space size */
    unsigned long freeswap;  /* Swap space still available */
    unsigned short procs;    /* Number of current processes */
    char _f[20-2*sizeof(long)-sizeof(int)];  /* Pads structure to 64 bytes */
};

struct timeval
{
    long int tv_sec;		/* Seconds.  */
    long int tv_usec;	/* Microseconds.  */
};

struct rusage {
    struct timeval ru_utime; /* user CPU time used */
    struct timeval ru_stime; /* system CPU time used */
    long   ru_maxrss;        /* maximum resident set size */
    long   ru_ixrss;         /* integral shared memory size */
    long   ru_idrss;         /* integral unshared data size */
    long   ru_isrss;         /* integral unshared stack size */
    long   ru_minflt;        /* page reclaims (soft page faults) */
    long   ru_majflt;        /* page faults (hard page faults) */
    long   ru_nswap;         /* swaps */
    long   ru_inblock;       /* block input operations */
    long   ru_oublock;       /* block output operations */
    long   ru_msgsnd;        /* IPC messages sent */
    long   ru_msgrcv;        /* IPC messages received */
    long   ru_nsignals;      /* signals received */
    long   ru_nvcsw;         /* voluntary context switches */
    long   ru_nivcsw;        /* involuntary context switches */
};

struct tm
{
    int	tm_sec;
    int	tm_min;
    int	tm_hour;
    int	tm_mday;
    int	tm_mon;
    int	tm_year;
    int	tm_wday;
    int	tm_yday;
    int	tm_isdst;
#ifdef __TM_GMTOFF
    long	__TM_GMTOFF;
#endif
#ifdef __TM_ZONE
    const char *__TM_ZONE;
#endif
};

#endif //OS_RISC_V_POSIX_STRUCTS_H
