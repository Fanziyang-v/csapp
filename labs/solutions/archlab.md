# Architecture lab

> **Note**: Updated to Y86-64 for CS:APP3e. Students are given a small default Y86-64 array copying function and a working pipelined Y86-64 processor design that runs the copy function in some nominal number of *clock cycles per array element* (CPE). The students attempt to minimize the CPE by modifying both the function and the processor design. This gives the students a deep appreciation for the interactions between hardware and software.

注：在 [CSAPP 官网](https://csapp.cs.cmu.edu/3e/labs.html) 中，可以下载所有不含具体答案的 CSAPP 实验。

本文档分为两个部分：
1. Introduction 部分介绍了 archlab 的实验内容、实验目标。
2. Solutions 部分给出每一部分的分析和解答。


## Introduction
In this lab, you will learn about the design and implementation of a pipelined Y86-64 processor, optimizing both it and a benchmark program to maximize performance. You are allowed to make any semantics-preserving transformation to the benchmark program, or to make enhancements to the pipelined processor,
or both. When you have completed the lab, you will have a keen appreciation for the interactions between code and hardware that affect the performance of your programs. 

The lab is organized into three parts. In Part A you will write some simple Y86-64 programs and become familiar with the Y86-64 tools. In Part B, you will extend the SEQ simulator with a new instruction. These two parts will prepare you for Part C, the heart of the lab, where you will optimize the Y86-64 benchmark program and the processor design.


## Solutions

### Part A
这一部分，我们需要在 sim/misc 目录下完成。我们的任务是编写和模拟三个 Y86-64 程序，这些程序对应的 C 语言代码位于 [example.c](/labs/archlab/sim/misc/examples.c) 中，在编写好 Y86-64 程序后，先用 YAS 工具进行汇编，再用 YIS 工具进行模拟。

编写 Y86-64 程序之前，先介绍以下 Y86-64 与 x86-64 程序的区别以及可用的指令。首先，Y86-64 程序可用的指令共 10 种类型，如下表所示:
| 指令 | 功能 |
| --- | --- |
| halt | 终止程序 |
| nop | no operation |
| rrmovq rA, rB | R[rB] <- R[rA] |
| irmovq V, rB | R[rB] <- V |
| rmmovq rA, D(rB) | M[R[rB] + D] <- R[rA] |
| Opq rA, rB | R[rB] <- R[rB] Op R[rA] |
| jXX Dest | 跳转 |
| cmovXX | 条件传送 |
| call Dest | 函数调用 |
| ret | 函数返回 |
| pushq rA| 将 R[rA] 压栈 |
| popq rA | 将栈顶 8 字节弹栈，存放于 rA 中 |

其中 Op 支持四种算术逻辑运算，分别为：andq（与）、xorq（异或）、addq（加）、subq（减）。

Y86-64 程序与 x86-64 程序的某些区别在于：
1. 所有的算术和逻辑运算，都发生在寄存器之间。
2. 寻址模式简单，只有**立即数寻址**或者**基址+偏移量**共两种寻址方式。
3. 算术逻辑运算中源操作数只能是寄存器，不可以是立即数。
4. 指令种类少，在 Y86-64 中，只能使用有限的指令去实现目标功能。

#### sum.ys
编写 Y86-64 程序 sum.ys，通过迭代的方式对链表所有元素求和，我们的程序应该包含**设置栈结构、调用函数，然后终止程序**。该程序的行为等同于函数 `sum_list`：
```c
/* linked list element */
typedef struct ELE {
    long val;
    struct ELE *next;
} *list_ptr;

/* sum_list - Sum the elements of a linked list */
long sum_list(list_ptr ls)
{
    long val = 0;
    while (ls) {
	val += ls->val;
	ls = ls->next;
    }
    return val;
}
```

函数 `sum_list` 对应的 Y86-64 代码如下：
```
sum_list:
    xorq %rax, %rax         # val = 0
    jmp test                # Goto test
loop:
    mrmovq (%rdi), %rdx     # Get ls->val
    addq %rdx, %rax         # val += ls->val
    mrmovq 8(%rdi), %rdi    # ls = ls->next
test:
    andq %rdi, %rdi         # Set CC
    jne loop                # If !=, goto loop
    ret                     # return val
```

整体的 Y86-64 程序如下，包含栈结构的设置、函数调用、样例数据设定以及程序终止：
```
# Execution begins at address 0
        .pos 0
        irmovq stack, %rsp  # Set up stack pointer
        call main           # Execute main program
        halt                # Terminate Program

# Sample linked list
        .align 8
ele1:
        .quad 0x00a
        .quad ele2
ele2:
        .quad 0x0b0
        .quad ele3
ele3:
        .quad 0xc00
        .quad 0

main:
    irmovq ele1, %rdi
    call sum_list           # sum_list(ls)
    ret

# long sum_list(list_ptr ls)
# ls in %rdi
sum_list:
    xorq %rax, %rax         # val = 0
    jmp test                # Goto test
loop:
    mrmovq (%rdi), %rdx     # Get ls->val
    addq %rdx, %rax         # val += ls->val
    mrmovq 8(%rdi), %rdi    # ls = ls->next
test:
    andq %rdi, %rdi         # Set CC
    jne loop                # If !=, goto loop
    ret                     # return val


# Stack starts here and grows to lower addresses
        .pos 0x200
stack:
```
使用 YAS 将 sum.ys 进行汇编，然后使用 YIS 进行模拟：
```
./yas sum.ys
./yis sum.yo
Stopped in 26 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rdx:   0x0000000000000000      0x0000000000000c00
%rsp:   0x0000000000000000      0x0000000000000200

Changes to memory:
0x01f0: 0x0000000000000000      0x000000000000005b
0x01f8: 0x0000000000000000      0x0000000000000013
```
可见 %rax 的值变为 0xcba，说明求和结果正确。

#### rsum.ys
接下来需要实现 `sum_list` 的递归版本 `rsum_list` 的 Y86-64 程序。
```c
/* rsum_list - Recursive version of sum_list */
long rsum_list(list_ptr ls)
{
    if (!ls)
	    return 0;
    else {
        long val = ls->val;
        long rest = rsum_list(ls->next);
        return val + rest;
    }
}
```
对应的 Y86-64 代码如下：
```
rsum_list:
    pushq %rbx              # Save %rbx
    xorq %rax, %rax         # val = 0
    andq %rdi, %rdi         # Set CC
    je end                  # if =, goto end
    mrmovq (%rdi), %rbx     # val = ls->val
    mrmovq 8(%rdi), %rdi    # Get ls->next
    call rsum_list          # call rsum_list(ls->next)
    addq %rbx, %rax         # val += rest
end:
    popq %rbx               # Restore %rbx
    ret                     # return val
```

整体的 Y86-64 程序如下，包含栈结构的设置、函数调用、样例数据设定以及程序终止：
```
# Execution begins at address 0
        .pos 0
        irmovq stack, %rsp  # Set up stack pointer
        call main           # Execute main program
        halt                # Terminate Program

# Sample linked list
        .align 8
ele1:
        .quad 0x00a
        .quad ele2
ele2:
        .quad 0x0b0
        .quad ele3
ele3:
        .quad 0xc00
        .quad 0

main:
    irmovq ele1, %rdi
    call rsum_list           # sum_list(ls)
    ret

# long rsum_list(list_ptr ls)
# ls in %rdi
rsum_list:
    pushq %rbx              # Save %rbx
    xorq %rax, %rax         # val = 0
    andq %rdi, %rdi         # Set CC
    je end                  # if =, goto end
    mrmovq (%rdi), %rbx     # val = ls->val
    mrmovq 8(%rdi), %rdi    # Get ls->next
    call rsum_list          # call rsum_list(ls->next)
    addq %rbx, %rax         # val += rest
end:
    popq %rbx               # Restore %rbx
    ret                     # return val


# Stack starts here and grows to lower addresses
        .pos 0x200
stack:
```
使用 YAS 将 sum.ys 进行汇编，然后使用 YIS 进行模拟：
```
./yas rsum.ys
./yis rsum.yo
Stopped in 42 steps at PC = 0x13.  Status 'HLT', CC Z=0 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200

Changes to memory:
0x01b8: 0x0000000000000000      0x0000000000000c00
0x01c0: 0x0000000000000000      0x0000000000000088
0x01c8: 0x0000000000000000      0x00000000000000b0
0x01d0: 0x0000000000000000      0x0000000000000088
0x01d8: 0x0000000000000000      0x000000000000000a
0x01e0: 0x0000000000000000      0x0000000000000088
0x01f0: 0x0000000000000000      0x000000000000005b
0x01f8: 0x0000000000000000      0x0000000000000013
```

#### copy.ys
接下来实现函数 `copy_block` 的 Y86-64 程序，对应的 C 语言代码如下：
```c
/* copy_block - Copy src to dest and return xor checksum of src */
long copy_block(long *src, long *dest, long len)
{
    long result = 0;
    while (len > 0) {
        long val = *src++;
        *dest++ = val;
        result ^= val;
        len--;
    }
    return result;
}
```
对应的 Y86-64 代码如下：
```
# src in %rdi, dest in %rsi, len in %rdx
copy_block:
    pushq %rbx
    irmovq $8, %r8
    irmovq $1, %r9
    xorq %rax, %rax
    andq %rdx, %rdx
    jmp test
loop:
    mrmovq (%rdi), %rbx
    rmmovq %rbx, (%rsi)
    xorq %rbx, %rax
    addq %r8, %rdi
    addq %r8, %rsi
    subq %r9, %rdx
test:
    jg loop
    popq %rbx
    ret
```
整体的 Y86-64 程序如下，包含栈结构的设置、函数调用、样例数据设定以及程序终止：
```
# Execution begins at address 0
        .pos 0
        irmovq stack, %rsp  # Set up stack pointer
        call main           # Execute main program
        halt                # Terminate Program

.align 8
# Source block
src:
        .quad 0x00a
        .quad 0x0b0
        .quad 0xc00
# Destination block
dest:
        .quad 0x111
        .quad 0x222
        .quad 0x333

main:
    irmovq src, %rdi
    irmovq dest, %rsi
    irmovq $3, %rdx
    call copy_block           # copy_block(src, dest, len)
    ret

# void copy_block(long *src, long *dest, long len)
# src in %rdi, dest in %rsi, len in %rdx
copy_block:
    pushq %rbx
    irmovq $8, %r8
    irmovq $1, %r9
    xorq %rax, %rax
    andq %rdx, %rdx
    jmp test
loop:
    mrmovq (%rdi), %rbx
    rmmovq %rbx, (%rsi)
    xorq %rbx, %rax
    addq %r8, %rdi
    addq %r8, %rsi
    subq %r9, %rdx
test:
    jg loop
    popq %rbx
    ret

# Stack starts here and grows to lower addresses
        .pos 0x200
stack:

```
使用 YAS 对该 Y86-64 程序进行汇编，然后使用 YIS 进行模拟：
```
./yas copy.ys
./yis copy.yo
Stopped in 38 steps at PC = 0x13.  Status 'HLT', CC Z=1 S=0 O=0
Changes to registers:
%rax:   0x0000000000000000      0x0000000000000cba
%rsp:   0x0000000000000000      0x0000000000000200
%rsi:   0x0000000000000000      0x0000000000000048
%rdi:   0x0000000000000000      0x0000000000000030
%r8:    0x0000000000000000      0x0000000000000008
%r9:    0x0000000000000000      0x0000000000000001

Changes to memory:
0x0030: 0x0000000000000111      0x000000000000000a
0x0038: 0x0000000000000222      0x00000000000000b0
0x0040: 0x0000000000000333      0x0000000000000c00
0x01f0: 0x0000000000000000      0x000000000000006f
0x01f8: 0x0000000000000000      0x0000000000000013
```
可见复制成功。
