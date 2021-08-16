# 我们将会用一个宏来用循环保存寄存器。这是必要的设置
.altmacro
# 寄存器宽度对应的字节数
.set    REG_SIZE, 8
# Context 的大小
.set    CONTEXT_SIZE, 37
.set    KERNEL_STACK_SIZE, 4096

# 宏：将寄存器存到栈上
.macro SAVE reg, offset
    sd  \reg, \offset*REG_SIZE(sp)
.endm

.macro SAVE_N n
    SAVE  x\n, \n
.endm


# 宏：将寄存器从栈中取出
.macro LOAD reg, offset
    ld  \reg, \offset*REG_SIZE(sp)
.endm

.macro LOAD_N n
    LOAD  x\n, \n
.endm

    .section .align.interrupt
    .global __interrupt
    .extern handle_interrupt
    .extern debug_func
# 进入中断
__interrupt:
    #保存原先的栈顶指针到sscratch
    csrw sscratch, sp

    # 在栈上开辟 Context 所需的空间
    li      sp, 0x80000000+8*1024*1024-8
    addi    sp, sp, -CONTEXT_SIZE*8

    # 保存通用寄存器，除了 x0（固定为 0）
    SAVE    x1, 1
    # 将原来的 sp（sp 又名 x2）写入 2 位置
    csrr    x1, sscratch
    SAVE    x1, 2
    # 保存 x3 至 x31
    .set    n, 3
    .rept   29
        SAVE_N  %n
        .set    n, n + 1
    .endr

    # 取出 CSR 并保存
    csrr    s1, sstatus
    csrr    s2, sepc
    csrr    s3, stval
    csrr    s4, scause
    csrr    s5, satp
    SAVE    s1, 32
    SAVE    s2, 33
    SAVE    s3, 34
    SAVE    s4, 35
    li      s4, 0x8000000000000000
    or      s5, s5, s4
    SAVE    s5, 36

    la t0, kernelContext
    # kernel_satp
    # ld t1, 0(t0)
    # csrw satp, t1
    # kernel_handle_interrupt
    ld t2, 16(t0)

    # sfence.vma zero,zero

    # 调用 handle_interrupt，传入参数
    # context: &mut Context
    mv      a0, sp
    # scause: Scause
    csrr    a1, scause
    # stval: usize
    csrr    a2, stval

    # jal  handle_interrupt
    jr t2

    .global __restore
# 离开中断
# 从 Context 中恢复所有寄存器，并跳转至 Context 中 sepc 的位置
__restore:

    # receive Context
    li      sp, 0x80000000+8*1024*1024-8
    addi    sp, sp, -CONTEXT_SIZE*8
    # 恢复 CSR
    LOAD    s1, 32
    LOAD    s2, 33
    LOAD    s3, 34
    LOAD    s4, 35
    LOAD    s5, 36
    csrw    sstatus, s1
    csrw    sepc, s2
    csrw    stval, s3
    csrw    scause, s4
    csrw    satp, s5

    sfence.vma zero, zero

    # 恢复通用寄存器
    LOAD    x1, 1
    # 恢复 x3 至 x31
    .set    n, 3
    .rept   29
        LOAD_N  %n
        .set    n, n + 1
    .endr

    # 恢复 sp（又名 x2）这里最后恢复是为了上面可以正常使用 LOAD 宏
    LOAD    x2, 2

    sret

.global __fast_restore
# 离开中断
# 从 Context 中恢复所有寄存器，并跳转至 Context 中 sepc 的位置
__fast_restore:

    # receive Context
    li      sp, 0x80000000+8*1024*1024-8
    addi    sp, sp, -CONTEXT_SIZE*8
    # 恢复 CSR
    LOAD    s1, 32
    LOAD    s2, 33
    LOAD    s3, 34
    LOAD    s4, 35
    LOAD    s5, 36
    csrw    sstatus, s1
    csrw    sepc, s2
    csrw    stval, s3
    csrw    scause, s4
    csrw    satp, s5

    # sfence.vma zero, zero

    # 恢复通用寄存器
    LOAD    x1, 1
    # 恢复 x3 至 x31
    .set    n, 3
    .rept   29
        LOAD_N  %n
        .set    n, n + 1
    .endr

    # 恢复 sp（又名 x2）这里最后恢复是为了上面可以正常使用 LOAD 宏
    LOAD    x2, 2

    sret