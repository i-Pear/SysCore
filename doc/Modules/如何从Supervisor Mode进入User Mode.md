# 如何从Supervisor Mode进入User Mode

在Risc-V中，Mode切换靠的是一系列的ret指令。

比如从M态进入S态靠的是mret。

从S态进入U态靠的是sret。

因为我们使用了rustsbi，所以我们已经在S态了，那么我们如何进入U态呢？

我们知道： 中断后硬件会执行以下动作：
* 异常指令的pc被保存在sepc中;
* pc被设置为stvec;
* scause被设置为异常原因;
* stval被设置为出错地址或其他信息;
* sie置零以禁止中断;之前的sie被保存到spie;
* 之前的权限模式被保存到spp中;
  

之后的流程是我们手动完成的：
* 保存现场;
* 跳到中断处理函数;
*  恢复现场;

所以这里需要恢复现场+将模拟硬件自动完成的动作。

```c
// 上下文对象，我们保存在这里的寄存器将会在中断恢复现场时被写入真实寄存器
Context* thread_context = malloc(sizeof(Context));
// 读取当前sstatus
thread_context->sstatus = register_read_sstatus();
// 分配栈指针
thread_context->sp = alloc_page(4096) + __page_size - 2 * 8;
// 此处spp应为0,表示user-mode
thread_context->sstatus |= REGISTER_SSTATUS_SPP; // spp = 1
thread_context->sstatus ^= REGISTER_SSTATUS_SPP; // spp = 0
// 此处spie应为1,表示user-mode允许中断
thread_context->sstatus |= REGISTER_SSTATUS_SPIE; // spie = 1
// 此处sepc为中断后返回地址
thread_context->sepc = entry;
/**
 * 页表处理
 * 1. satp应由物理页首地址右移12位并且或上（8 << 60），表示开启sv39分页模式
 * 2. 未使用的页表项应该置0
 */
size_t page_table_base = (size_t) alloc_page(4096);
memset(page_table_base,0,4096);
// 0x8000_0000 -> 0x8000_0000
*((size_t *) page_table_base + 2) = (0x80000 << 10) | 0xdf;
thread_context->satp = (page_table_base>>12)|(8LL << 60);
// 进入中断恢复处理函数
__restore(context);
```

