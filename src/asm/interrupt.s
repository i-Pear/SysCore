# 我们将会用一个宏来用循环保存寄存器。这是必要的设置
.altmacro
# 寄存器宽度对应的字节数
.set    REG_SIZE, 8
# Context 的大小
.set    CONTEXT_SIZE, 34

# 宏：将寄存器存到栈上
.macro SAVE reg, offset
    sd  \reg, \offset*8(sp)
.endm

.macro SAVE_N n
    SAVE  x\n, \n
.endm


# 宏：将寄存器从栈中取出
.macro LOAD reg, offset
    ld  \reg, \offset*8(sp)
.endm

.macro LOAD_N n
    LOAD  x\n, \n
.endm

    .section .text
    .global __interrupt
    .extern handle_interrupt
    .extern debug_func
# 进入中断
__interrupt:
    # swap sp & sscratch, 进入内核栈
    csrrw sp, sscratch, sp

    # 在栈上开辟 Context 所需的空间
    addi    sp, sp, -35*8

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
    SAVE    s1, 32
    csrr    s1, sepc
    SAVE    s1, 33
    csrr    s1, satp
    SAVE    s1, 34

    ld t0, 35*8(sp)
    csrw satp, t0
    sfence.vma

    # 调用 handle_interrupt，传入参数
    # context: &mut Context
    mv      a0, sp
    # scause: Scause
    csrr    a1, scause
    # PERHAPS BROKEN: scause's value is 0x8000000000000005????, so try to fix
    andi a1, a1, 31
    # stval: usize
    csrr    a2, stval
    # la ra, __restore
    # jal  handle_interrupt
    call handle_interrupt

    .global __restore
    .global __turn_to_user_mode
# 离开中断
# 从 Context 中恢复所有寄存器，并跳转至 Context 中 sepc 的位置
__restore:
    # receive Context
    mv sp, a0
    # 恢复 CSR
    LOAD    s1, 32
    LOAD    s2, 33
    csrw    sstatus, s1
    csrw    sepc, s2
    LOAD    s1, 34
    csrw    satp, s1

    sfence.vma

    # 恢复通用寄存器
    LOAD    x1, 1
    # 恢复 x3 至 x31
    .set    n, 3
    .rept   29
        LOAD_N  %n
        .set    n, n + 1
    .endr

    addi    sp, sp, 35*8


    addi    sp, sp, -8
    sd t0, -8(sp)
    csrr t0, satp
    sd t0, 0(sp)
    ld t0, -8(sp)
    # 恢复 sp（又名 x2）这里最后恢复是为了上面可以正常使用 LOAD 宏
    csrrw sp, sscratch, sp

    sret

__turn_to_user_mode:
    # store sp
    addi sp, sp, -8
    csrr t0, satp
    sd t0, 0(sp)
    csrw sscratch, sp
    # receive Context
    mv sp, a0

    # 恢复 CSR
    LOAD    s1, 32
    LOAD    s2, 33
    csrw    sstatus, s1
    csrw    sepc, s2
    LOAD    s1, 34
    csrw    satp, s1

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

    # TODO  死在这了！
    sfence.vma

    sret
