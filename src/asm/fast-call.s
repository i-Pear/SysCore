#
#  Assemblies for User-Space Fast Syscall
#

    .section .f_syscall

    .global fast_syscall
    .global fast_syscall_getppid
    .global fast_syscall_clock_gettime
    .global f_syscall_get_start

    .extern f_syscall_start

# Here is user-space memory 0x2 0000 0000

fast_syscall:
    # input context is with a7 = Syscall_ID
    # a0 can be modified
    li a0,173 # SYS_getppid
    beq a0,a7,fast_syscall_getppid

fast_syscall_getppid:
    # a0 <- running.ppid
    li a0, 0x80000000+8*1024*1024-4         # place for ppid
    lw a0,0(a0)                             # get ppid
    ret

fast_syscall_clock_gettime:
    nop

f_syscall_get_start:
    la a0, f_syscall_start
    ret
