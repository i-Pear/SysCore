# SysCore的页表处理

## 页表介绍

在RiscV中页表结构如下：

![img.png](https://gitlab.eduxiji.net/iPear/syscore/-/raw/main/doc/Modules/assets/img.png)

- V 位决定了该页表项的其余部分是否有效（V = 1 时有效）。若 V = 0，则任何遍历
到此页表项的虚址转换操作都会导致页错误。
- R、W 和 X 位分别表示此页是否可以读取、写入和执行。如果这三个位都是 0，
那么这个页表项是指向下一级页表的指针，否则它是页表树的一个叶节点。
- U 位表示该页是否是用户页面。若 U = 0，则 U 模式不能访问此页面，但 S 模式
可以。若 U = 1，则 U 模式下能访问这个页面，而 S 模式不能。（可以通过设置sstatus寄存器sum位的方式使得内核态可访问用户态页面）
- G 位表示这个映射是否对所有虚址空间有效，硬件可以用这个信息来提高地址转
换的性能。这一位通常只用于属于操作系统的页面。
- A 位表示自从上次 A 位被清除以来，该页面是否被访问过。
- D 位表示自从上次清除 D 位以来页面是否被弄脏（例如被写入）。
- RSW 域留给操作系统使用，它会被硬件忽略。
- PPN 域包含物理页号，这是物理地址的一部分。若这个页表项是一个叶节点，那
么 PPN 是转换后物理地址的一部分。否则 PPN 给出下一节页表的地址。

## 页表处理

内核态起始拥有一个页表，写在boot.s之中。用这个页表来完成内核初始化等流程，在创建用户程序之后便会抛弃此内核页表。

[0x0000_0000, 0x4000_0000) 用于映射端口

[0x4000_0000, 0x8000_0000) 用于映射端口

[0x8000_0000, 0xc000_0000) 用于映射真实内核地址

```asm
boot_page_table:
    # 第 0 项: 0x0000_0000 -> 0x0000_0000, 0xcf 表示 VRWXAD 均为 1, 只允许S态访问
    .quad (0x00000 << 10) | 0xcf
    # .quad (0x00000 << 10) | 0xcf
    # 第 1 项: 0x4000_0000 -> 0x4000_0000, 0xcf 表示 VRWXAD 均为 1, 只允许S态访问
    .quad (0x40000 << 10) | 0xcf
    # 第 2 项：0x8000_0000 -> 0x8000_0000，0xcf 表示 VRWXAD 均为 1, 只允许S态访问
    .quad (0x80000 << 10) | 0xcf
    .zero 507 * 8
    .quad 0
    .quad 0
```

每个用户态程序在创建时都会完整的拥有内核所需的所有页表。

```c++
// 0x3800_1000 -> 0x3800_1000 (4K)
PageTableUtil::CreateMapping(
        page_table_base,
        0x38001000,
        0x38001000,
        PAGE_TABLE_LEVEL::SMALL,
        PRIVILEGE_LEVEL::SUPERVISOR);
// 0x4000_0000 -> 0c4000_0000 (1G)
PageTableUtil::CreateMapping(
        page_table_base,
        0x40000000,
        0x40000000,
        PAGE_TABLE_LEVEL::LARGE,
        PRIVILEGE_LEVEL::SUPERVISOR);
// 0x8000_0000 -> 0x8000_0000 (1G)
PageTableUtil::CreateMapping(
        page_table_base,
        0x80000000,
        0x80000000,
        PAGE_TABLE_LEVEL::LARGE,
        PRIVILEGE_LEVEL::USER);
```

之后从用户态进入内核态时内核态直接使用用户空间的页表，既不用进行页表切换也不用数据交换，提高了内核运行的速度。

