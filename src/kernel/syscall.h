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
#define SYS_clock_nanosleep 115
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


#define SYS_setitimer 103
#define SYS_clock_settime 112
#define SYS_clock_gettime 113
#define SYS_syslog 116
#define SYS_sched_setaffinity 122
#define SYS_sched_getaffinity 123
#define SYS_sched_get_priority_max 125
#define SYS_sched_get_priority_min 126
#define SYS_kill 129
#define SYS_tkill 130
#define SYS_tgkill 131
#define SYS_rt_sigaction 134
#define SYS_rt_sigprocmask 135
#define SYS_rt_sigreturn 139
#define SYS_removexattr 14
#define SYS_setpriority 140
#define SYS_getpriority 141
#define SYS_reboot 142
#define SYS_setgid 144
#define SYS_setuid 146
#define SYS_getresuid 148
#define SYS_lremovexattr 15
#define SYS_getresgid 150
#define SYS_setpgid 154
#define SYS_getpgid 155
#define SYS_getsid 156
#define SYS_setsid 157
#define SYS_getgroups 158
#define SYS_setgroups 159
#define SYS_sethostname 161
#define SYS_getrlimit 163
#define SYS_setrlimit 164
#define SYS_getrusage 165
#define SYS_umask 166
#define SYS_prctl 167
#define SYS_adjtimex 171
#define SYS_getuid 174
#define SYS_geteuid 175
#define SYS_getgid 176
#define SYS_getegid 177
#define SYS_gettid 178
#define SYS_sysinfo 179
#define SYS_msgget 186
#define SYS_msgctl 187
#define SYS_semget 190
#define SYS_semctl 191
#define SYS_semop 193
#define SYS_shmget 194
#define SYS_shmctl 195
#define SYS_shmat 196
#define SYS_shmdt 197
#define SYS_socket 198
#define SYS_socketpair 199
#define SYS_bind 200
#define SYS_listen 201
#define SYS_getsockname 204
#define SYS_getpeername 205
#define SYS_setsockopt 208
#define SYS_getsockopt 209
#define SYS_shutdown 210
#define SYS_readahead 213
#define SYS_mremap 216
#define SYS_swapon 224
#define SYS_swapoff 225
#define SYS_mprotect 226
#define SYS_mlock 228
#define SYS_munlock 229
#define SYS_madvise 233
#define SYS_fcntl 25
#define SYS_prlimit64 261
#define SYS_clock_adjtime 266
#define SYS_syncfs 267
#define SYS_setns 268
#define SYS_renameat2 276
#define SYS_ioctl 29
#define SYS_flock 32
#define SYS_mknodat 33
#define SYS_symlinkat 36
#define SYS_pivot_root 41
#define SYS_statfs 43
#define SYS_ftruncate 46
#define SYS_fallocate 47
#define SYS_faccessat 48
#define SYS_setxattr 5
#define SYS_fchdir 50
#define SYS_chroot 51
#define SYS_fchmod 52
#define SYS_fchmodat 53
#define SYS_fchownat 54
#define SYS_fchown 55
#define SYS_lsetxattr 6
#define SYS_lseek 62
#define SYS_readv 65
#define SYS_writev 66
#define SYS_sendfile 71
#define SYS_ppoll 73
#define SYS_readlinkat 78
#define SYS_fstatat 79
#define SYS_sync 81
#define SYS_utimensat 88
#define SYS_acct 89
#define SYS_capget 90
#define SYS_capset 91
#define SYS_personality 92
#define SYS_exit_group 94
#define SYS_set_tid_address 96
#define SYS_unshare 97
#define SYS_futex 98
#define SYS_pselect6 72
#define SYS_prlimit64 261



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