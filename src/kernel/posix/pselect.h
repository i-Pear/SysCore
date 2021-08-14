#ifndef __PSELECT_H__
#define __PSELECT_H__
#ifdef __cplusplus
extern "C" {
#endif

#define __NFDBITS (8 * sizeof(unsigned long))                //每个ulong型可以表示多少个bit,
#define __FD_SETSIZE 1024                                          //socket最大取值为1024
#define __FDSET_LONGS (__FD_SETSIZE/__NFDBITS)     //bitmap一共有1024个bit，共需要多少个ulong

typedef struct {
    unsigned long fds_bits [__FDSET_LONGS];                 //用ulong数组来表示bitmap
} __kernel_fd_set;

typedef __kernel_fd_set   fd_set;

//每个ulong为32位，可以表示32个bit。
//fd  >> 5 即 fd / 32，找到对应的ulong下标i；fd & 31 即fd % 32，找到在ulong[i]内部的位置

#define FD_SET(fd, fdsetp)   (((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] |= (1<<((fd) & 31)))             //设置对应的bit
#define FD_CLR(fd, fdsetp)   (((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] &= ~(1<<((fd) & 31)))            //清除对应的bit
#define FD_ISSET(fd, fdsetp)   ((((fd_set *)(fdsetp))->fds_bits[(fd) >> 5] & (1<<((fd) & 31))) != 0)     //判断对应的bit是否为1
#define FD_ZERO(fdsetp)   (memset (fdsetp, 0, sizeof (*(fd_set *)(fdsetp))))                             //memset bitmap

#ifdef __cplusplus
}
#endif
#endif