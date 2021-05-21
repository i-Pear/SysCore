# 操作系统启动时所需的指令以及字段
#
# 我们在 linker.ld 中将程序入口设置为了 _start，因此在这里我们将填充这个标签
# 它将会执行一些必要操作，然后跳转至我们用 rust 编写的入口函数
#
# 关于 RISC-V 下的汇编语言，可以参考 https://github.com/riscv/riscv-asm-manual/blob/master/riscv-asm.md

    .section .text.entry
    .globl _start

    .global sbi_call
    .global read_time
    .global close_interrupt
    .global get_kernel_end
    .global get_boot_page_table
    .global flush_tlb
    .global interrupt_timer_init
    .global read_sp

    .extern __interrupt
    .extern set_next_timeout
    .extern shutdown
    
_start:
    # 计算 boot_page_table 的物理页号
    la t0, boot_page_table
    srli t0, t0, 12
    # 8 << 60 是 satp 中使用 Sv39 模式的记号
    li t1, (8 << 60)
    or t0, t0, t1
    # 写入 satp 并更新 TLB
    csrw satp, t0
    sfence.vma


    la sp, boot_stack_top

# Init interrupt
    la t0, __interrupt
    csrw stvec, t0
# Enable interrupt, But it looks no use, Even cause interrupt
    # csrsi mstatus, 0x8

# ebreak test
    # ebreak

    call main
    call shutdown
hlt:
    j hlt

sbi_call:
    addi sp,sp,-16
    sd ra,8(sp)

    mv a7, a0
    mv a0, a1
    mv a1, a2
    mv a2, a3
    ecall

    ld ra,8(sp)
    addi sp,sp,16
    ret

get_kernel_end:
    la a0, kernel_end
    ret

flush_tlb:
    sfence.vma
    ret

read_time:
    csrr a0, time
    ret

read_sp:
    addi sp,sp,-8
    sd ra,8(sp)

    mv a0, sp

    ld ra,8(sp)
    addi sp,sp,8
    ret

get_boot_page_table:
    la a0, boot_page_table
    ret

close_interrupt:
    csrwi sie, 0
    ret

# Init Timer interrupt
interrupt_timer_init:
    addi sp,sp,-8
    sd ra,8(sp) 

    # 开启时钟中断
    li t0, 1 << 5
    csrs sie, t0
    # 全局s-mode中断位
    csrsi sstatus, 2
    call set_next_timeout

    ld ra,8(sp)
    addi sp,sp,8
    ret

    # 回忆：bss 段是 ELF 文件中只记录长度，而全部初始化为 0 的一段内存空间
    # 这里声明字段 .bss.stack 作为操作系统启动时的栈
    .section .bss.stack
    .global boot_stack
boot_stack:
    # 16K 启动栈大小
    .space 4096 * 16
    .global boot_stack_top
boot_stack_top:
    # 栈结尾

# 初始内核映射所用的页表
    .section .data
    .align 12

boot_page_table:
    # 第 0 项: 0x0000_0000 -> 0x0000_0000, 0xcf 表示 VRWXAD 均为 1, 只允许S态访问
    .quad (0x00000 << 10) | 0xcf
    # 第 1 项: 0x4000_0000 -> 0x4000_0000, 0xcf 表示 VRWXAD 均为 1, 只允许S态访问
    .quad (0x40000 << 10) | 0xcf
    # 第 2 项：0x8000_0000 -> 0x8000_0000，0xcf 表示 VRWXAD 均为 1, 只允许S态访问
    .quad (0x80000 << 10) | 0xcf
    .zero 507 * 8
    .quad 0
    .quad 0
