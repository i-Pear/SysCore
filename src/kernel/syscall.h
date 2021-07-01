#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "interrupt.h"


#define SYS_getcwd 17
#define SYS_dup 23
#define SYS_dup3 24
#define SYS_mkdirat 34
#define SYS_unlinkat 35
#define SYS_linkat 37
#define SYS_umount2 39
#define SYS_mount 40
#define SYS_chdir 49
#define SYS_openat 56
#define SYS_close 57
#define SYS_pipe2 59
#define SYS_getdents64 61
#define SYS_read 63
#define SYS_write 64
#define SYS_fstat 80
#define SYS_exit 93
#define SYS_nanosleep 101
#define SYS_sched_yield 124
#define SYS_times 153
#define SYS_uname 160
#define SYS_gettimeofday 169
#define SYS_getpid 172
#define SYS_getppid 173
#define SYS_brk 214
#define SYS_munmap 215
#define SYS_clone 220
#define SYS_execve 221
#define SYS_mmap 222
#define SYS_wait4 260


// 调用
/**
 * ```
static inline _u64 internal_syscall(long n, _u64 _a0, _u64 _a1, _u64 _a2, _u64
		_a3, _u64 _a4, _u64 _a5) {
	register _u64 a0 asm("a0") = _a0;
	register _u64 a1 asm("a1") = _a1;
	register _u64 a2 asm("a2") = _a2;
	register _u64 a3 asm("a3") = _a3;
	register _u64 a4 asm("a4") = _a4;
	register _u64 a5 asm("a5") = _a5;
	register long syscall_id asm("a7") = n;
	asm volatile ("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"
			(a5), "r"(syscall_id));
	return a0;
}
```
 */

#define SYS_getchar 996

Context* syscall(Context* context);

char *getAbsolutePath(char *path, char *cwd);

#endif