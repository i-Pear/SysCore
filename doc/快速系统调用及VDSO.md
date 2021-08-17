# Virtual Dynamic Shared Object



## 技术起源

由于使用ecall指令陷入中断（并保存、恢复寄存器等）会消耗较长的时间，Linux和glibc支持堆某些特殊的（主要是时间相关）系统调用，提供在用户态执行的方案。

使用VDSO。可以避免syscall陷入内核态，从而执行高速（数十倍速度）的体验。



## 实现方法

VDSO由一个操作系统提供的共享库实现。

在加载器初始化用户栈时，需要在用户栈中提前压入部分变量，用来标识系统信息（称为AUX）。其中有一项AT_SYSINFO_EHDR 可以指向共享库（.so）的ELF头部，glibc在接收到这个参数后会解析可用的VDSO系统调用。

VDSO库需要与Linux内核一起编译，具有特殊且复杂的编译参数。在本次比赛中我们跳过重新编译Linux内核的过程，转而修改已有Linux的VDSO，来适配我们的系统。



## VDSO构建细节

### 1.提取Linux VDSO

关闭Linux VSDO地址随机化：

```
sudo sysctl -w kernel.randomize_va_space=0
```

查看VDSO位置：

```
cat /proc/self/maps

3fc4681000-3fc4682000 r-xp 00000000 00:00 0           [vdso]
3fc4682000-3fc4683000 r--p 00000000 00:00 0           [vdso_data]
3fc4683000-3fc469b000 r-xp 00000000 fe:01 262334      /lib/riscv64-linux-gnu/ld-2.31.so
3fc469b000-3fc469c000 r--p 00017000 fe:01 262334      /lib/riscv64-linux-gnu/ld-2.31.so
3fc469c000-3fc469d000 rw-p 00018000 fe:01 262334      /lib/riscv64-linux-gnu/ld-2.31.so
```

另外可见vdso_text与vdso_data是连在一起的。从vdso_text开始是一个完整的ELF，部分数据存储在了vdso_data中。

dump VSDO页：

```
dd if=/proc/self/mem of=vdso_text.so bs=4096 skip=$[0x3fc4681] count=1
dd if=/proc/self/mem of=vdso_data.so bs=4096 skip=$[0x3fc4682] count=1
```

用objdump查看信息：

```
> objdump -x vdso_text.so

vdso_text.so:     file format elf64-little
vdso_text.so
architecture: UNKNOWN!, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x0000000000000800

Program Header:
    LOAD off    0x0000000000000000 vaddr 0x0000000000000000 paddr 0x0000000000000000 align 2**12
         filesz 0x0000000000000a60 memsz 0x0000000000000a60 flags r-x
 DYNAMIC off    0x0000000000000390 vaddr 0x0000000000000390 paddr 0x0000000000000390 align 2**3
         filesz 0x0000000000000130 memsz 0x0000000000000130 flags r--
    NOTE off    0x0000000000000350 vaddr 0x0000000000000350 paddr 0x0000000000000350 align 2**2
         filesz 0x000000000000003c memsz 0x000000000000003c flags r--
EH_FRAME off    0x00000000000004c0 vaddr 0x00000000000004c0 paddr 0x00000000000004c0 align 2**2
         filesz 0x0000000000000024 memsz 0x0000000000000024 flags r--

SYMBOL TABLE:
no symbols

```

### 2. 修改 VDSO

反编译得：

```
vdso.so:     file format elf64-littleriscv


Disassembly of section .text:

0000000000000800 <__vdso_rt_sigreturn@@LINUX_4.15>:
 800:	08b00893          	li	a7,139
 804:	00000073          	ecall
	...

000000000000080a <__vdso_clock_gettime@@LINUX_4.15>:
 80a:	1141                	addi	sp,sp,-16
 80c:	e422                	sd	s0,8(sp)
 80e:	0800                	addi	s0,sp,16
 810:	47bd                	li	a5,15
    ......
```

这里就是VSDO的运行库代码段，但是这部分代码是适配Linux的，需要进行替换。由于用户态无法访问S态内存，对于需要共享的数据要在页表上做特殊的处理（注意安全，尽量只读）。

这里只考虑clock_gettime的实现，这个系统调用并不需要与内核交互，我们只需要rdtime处理后直接返回即可。我们将自己的实现逻辑编译为二进制文件，并找到函数头位置，将整个函数覆盖掉Linux VDSO中的函数体。

### 3. 加载VDSO

在加载器初始化栈时加上一个AT_SYSINFO_EHDR

```
put_aux((size_t **) &sp, AT_SYSINFO_EHDR, 0x300000000);
```



## VDSO效果

未启用VDSO：

![](D:\Project\syscore\doc\assets\vdso.jpg)

启用VDSO：

可看到CLOCK_GETTIME已经全部被VDSO转发，不再请求ecall

![](D:\Project\syscore\doc\assets\no-vdso.jpg)
