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

详细的实验介绍请见 [archlab.pdf](/labs/archlab/archlab.pdf)，从 [CSAPP 官网](http://csapp.cs.cmu.edu/labs.html) 中下载到 archlab-handout.tar 后，执行以下指令，将实验文件解压，并且使用 make 命令构建 archlab 所需的工具如 YAS、YIS。
```
tar xvf archlab-handout.tar
mv archlab-handout archlab
cd archlab
tar xvf sim
cd sim
make clean; make
```
注：在执行 make 命令时可能会出现一些错误，这些错误请自行解决，本文档仅提供实验的解决方案。

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

### Part B
我们将在 sim/seq 下完成这一部分的实验，Part B 的任务就是扩展 SEQ 处理器，新增一条指令： iaddq V, rB，我们需要修改 [seq-full.hcl](/labs/archlab/sim/seq/seq-full.hcl) 文件。

我们先写出指令 iaddq 各阶段的微操作，如下表所示：
| 阶段 | iaddq V, rB |
| --- | --- |
| 取指 | icode:ifun <- M1[PC]<br> rA:rB <- M1[PC+1]<br> valC <- M8[PC+2]<br>valP <- PC+10 |
| 译码 | valB <- R[rB] |
| 执行 | valE <- valB + valC |
| 访存 | |
| 写回 | R[rA] <- valE |
| 更新PC | PC <- valP |

#### Fetch Stage
在取指阶段，将 IIADDQ 添加到 `instr_valid`、`need_regids`、`need_valC` 中，表示指令 iaddq 是合法的、需要寄存器字节、需要用到8字节常数。修改的内容如下所示：
```
bool instr_valid = icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	       IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ, IIADDQ };

# Does fetched instruction require a regid byte?
bool need_regids =
	icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };

# Does fetched instruction require a constant word?
bool need_valC =
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIADDQ };
```

#### Decode Stage
在译码阶段，我们不需要使用寄存器作为 srcA，因此不需要修改 srcA，但是需要使用寄存器 rB 作为 srcB、且作为 dstE，将 IIADDQ 添加进去，修改后的内容如下：
```
## What register should be used as the B source?
word srcB = [
	icode in { IOPQ, IRMMOVQ, IMRMOVQ， IIADDQ  } : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];

## What register should be used as the E destination?
word dstE = [
	icode in { IRRMOVQ } && Cnd : rB;
	icode in { IIRMOVQ, IOPQ, IIADDQ} : rB;
	icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];
```

#### Execute Stage
在执行阶段，我们需要将 valB 作为 aluB，将 valC 作为 aluA，因此把 IIADDQ 分别添加进去，并且 iaddq 指令会设置条件码，修改后的结果如下：
```
## Select input A to ALU
word aluA = [
	icode in { IRRMOVQ, IOPQ } : valA;
	icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : valC;
	icode in { ICALL, IPUSHQ } : -8;
	icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];

## Select input B to ALU
word aluB = [
	icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		      IPUSHQ, IRET, IPOPQ, IIADDQ } : valB;
	icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];

## Set the ALU function
word alufun = [
	icode == IOPQ : ifun;
	1 : ALUADD;
];

## Should the condition codes be updated?
bool set_cc = icode in { IOPQ, IIADDQ };
```
#### Memory Stage
在访存阶段，由于指令 iaddq 不涉及访存操作，因此这里无需进行修改。

#### PC Update Stage
在更新PC 阶段，PC 由 valP 进行修改，这是默认的 PC 更新规则，因此这部分也不需要修改。

#### Evaluation
修改完 [seq-full.hcl](/labs/archlab/sim/seq/seq-full.hcl) 文件后，运行以下指令，构建一个新的 SEQ 仿真器：
```
make VERSION=full
# Building the seq-full.hcl version of SEQ
../misc/hcl2c -n seq-full.hcl <seq-full.hcl >seq-full.c
gcc -Wall -O2  -I../misc  -o ssim \
        seq-full.c ssim.c ../misc/isa.c  -lm
```
首先在一个简单的 Y86-64 程序上测试，例如 asumi.yo：
```
./ssim -t ../y86-code/asumi.yo
Y86-64 Processor: seq-full.hcl
137 bytes of code read
IF: Fetched irmovq at 0x0.  ra=----, rb=%rsp, valC = 0x100
IF: Fetched call at 0xa.  ra=----, rb=----, valC = 0x38
Wrote 0x13 to address 0xf8
IF: Fetched irmovq at 0x38.  ra=----, rb=%rdi, valC = 0x18
IF: Fetched irmovq at 0x42.  ra=----, rb=%rsi, valC = 0x4
IF: Fetched call at 0x4c.  ra=----, rb=----, valC = 0x56
Wrote 0x55 to address 0xf0
IF: Fetched xorq at 0x56.  ra=%rax, rb=%rax, valC = 0x0
IF: Fetched andq at 0x58.  ra=%rsi, rb=%rsi, valC = 0x0
IF: Fetched jmp at 0x5a.  ra=----, rb=----, valC = 0x83
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched mrmovq at 0x63.  ra=%r10, rb=%rdi, valC = 0x0
IF: Fetched addq at 0x6d.  ra=%r10, rb=%rax, valC = 0x0
IF: Fetched iaddq at 0x6f.  ra=----, rb=%rdi, valC = 0x8
IF: Fetched iaddq at 0x79.  ra=----, rb=%rsi, valC = 0xffffffffffffffff
IF: Fetched jne at 0x83.  ra=----, rb=----, valC = 0x63
IF: Fetched ret at 0x8c.  ra=----, rb=----, valC = 0x0
IF: Fetched ret at 0x55.  ra=----, rb=----, valC = 0x0
IF: Fetched halt at 0x13.  ra=----, rb=----, valC = 0x0
32 instructions executed
Status = HLT
Condition Codes: Z=1 S=0 O=0
Changed Register State:
%rax:   0x0000000000000000      0x0000abcdabcdabcd
%rsp:   0x0000000000000000      0x0000000000000100
%rdi:   0x0000000000000000      0x0000000000000038
%r10:   0x0000000000000000      0x0000a000a000a000
Changed Memory State:
0x00f0: 0x0000000000000000      0x0000000000000055
0x00f8: 0x0000000000000000      0x0000000000000013
ISA Check Succeeds
```
可见测试通过，再使用基准测试程序测试我们的实现：
```
cd ../y86-code; make testssim
../seq/ssim -t asum.yo > asum.seq
../seq/ssim -t asumr.yo > asumr.seq
../seq/ssim -t cjr.yo > cjr.seq
../seq/ssim -t j-cc.yo > j-cc.seq
../seq/ssim -t poptest.yo > poptest.seq
../seq/ssim -t pushquestion.yo > pushquestion.seq
../seq/ssim -t pushtest.yo > pushtest.seq
../seq/ssim -t prog1.yo > prog1.seq
../seq/ssim -t prog2.yo > prog2.seq
../seq/ssim -t prog3.yo > prog3.seq
../seq/ssim -t prog4.yo > prog4.seq
../seq/ssim -t prog5.yo > prog5.seq
../seq/ssim -t prog6.yo > prog6.seq
../seq/ssim -t prog7.yo > prog7.seq
../seq/ssim -t prog8.yo > prog8.seq
../seq/ssim -t ret-hazard.yo > ret-hazard.seq
grep "ISA Check" *.seq
asum.seq:ISA Check Succeeds
asumr.seq:ISA Check Succeeds
cjr.seq:ISA Check Succeeds
j-cc.seq:ISA Check Succeeds
poptest.seq:ISA Check Succeeds
prog1.seq:ISA Check Succeeds
prog2.seq:ISA Check Succeeds
prog3.seq:ISA Check Succeeds
prog4.seq:ISA Check Succeeds
prog5.seq:ISA Check Succeeds
prog6.seq:ISA Check Succeeds
prog7.seq:ISA Check Succeeds
prog8.seq:ISA Check Succeeds
pushquestion.seq:ISA Check Succeeds
pushtest.seq:ISA Check Succeeds
ret-hazard.seq:ISA Check Succeeds
rm asum.seq asumr.seq cjr.seq j-cc.seq poptest.seq pushquestion.seq pushtest.seq prog1.seq prog2.seq prog3.seq prog4.seq prog5.seq prog6.seq prog7.seq prog8.seq ret-hazard.seq
```
基准测试通过，接下来测试除了 iaddq 指令外，其余指令是否有问题：
```
cd ../ptest; make SIM=../seq/ssim
./optest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 49 ISA Checks Succeed
./jtest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 64 ISA Checks Succeed
./ctest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 22 ISA Checks Succeed
./htest.pl -s ../seq/ssim 
Simulating with ../seq/ssim
  All 600 ISA Checks Succeed
```
可见，我们添加 iaddq 指令的同时，没有影响到其他指令的执行，然后再测试我们 iaddq 的实现。
```
cd ../ptest; make SIM=../seq/ssim TFLAGS=-i
./optest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 58 ISA Checks Succeed
./jtest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 96 ISA Checks Succeed
./ctest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 22 ISA Checks Succeed
./htest.pl -s ../seq/ssim -i
Simulating with ../seq/ssim
  All 756 ISA Checks Succeed
```
以上的测试均通过。

### Part C
我们将在 sim/pipe 目录下完成这一部分的实验，修改 [pipe-full.hcl](/labs/archlab/sim/pipe/pipe-full.hcl) 文件以添加指令 iaddq V, rB。

#### 添加指令 iaddq
##### Fetch Stage
在取值阶段，修改 `instr_valid`、`need_regids`、`need_valC`，将 IIADDQ 添加进去，修改的内容如下：
```
# Is instruction valid?
bool instr_valid = f_icode in 
	{ INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ,
	  IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ, IIADDQ };

# Does fetched instruction require a regid byte?
bool need_regids =
	f_icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, 
		     IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ };

# Does fetched instruction require a constant word?
bool need_valC =
	f_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL, IIADDQ };
```

##### Decode Stage
在译码阶段，同 seq-full.hcl 中的类似，iaddq 指令不需要使用 srcA，因此 srcA 不需要修改，但是需要将 rB 作为 srcB 以及 dstE，因此修改如下：
```
## What register should be used as the B source?
word d_srcB = [
	D_icode in { IOPQ, IRMMOVQ, IMRMOVQ, IIADDQ  } : D_rB;
	D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't need register
];

## What register should be used as the E destination?
word d_dstE = [
	D_icode in { IRRMOVQ, IIRMOVQ, IOPQ, IIADDQ } : D_rB;
	D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RRSP;
	1 : RNONE;  # Don't write any register
];
```

##### Execute Stage
在执行阶段，iaddq 需要将valC 作为 aluA，将 valB 作为 aluB，并且指令会设置条件码，因此修改如下：
```
## Select input A to ALU
word aluA = [
	E_icode in { IRRMOVQ, IOPQ } : E_valA;
	E_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IIADDQ } : E_valC;
	E_icode in { ICALL, IPUSHQ } : -8;
	E_icode in { IRET, IPOPQ } : 8;
	# Other instructions don't need ALU
];

## Select input B to ALU
word aluB = [
	E_icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, 
		     IPUSHQ, IRET, IPOPQ, IIADDQ } : E_valB;
	E_icode in { IRRMOVQ, IIRMOVQ } : 0;
	# Other instructions don't need ALU
];

## Should the condition codes be updated?
bool set_cc = E_icode in { IOPQ, IIADDQ } &&
	# State changes only during normal operation
	!m_stat in { SADR, SINS, SHLT } && !W_stat in { SADR, SINS, SHLT };
```

##### Memory Stage
iaddq 指令不涉及访存操作，因此不需要修改。

##### Evaluation
运行以下指令，构建 pipe-full.hcl 仿真器：
```
make VERSION=full
# Building the pipe-full.hcl version of PIPE
../misc/hcl2c -n pipe-full.hcl < pipe-full.hcl > pipe-full.c
gcc -Wall -O2 -g -fcommon  -I../misc  -o psim psim.c pipe-full.c \
        ../misc/isa.c  -lm
./gen-driver.pl -n 4 -f ncopy.ys > sdriver.ys
../misc/yas sdriver.ys
./gen-driver.pl -n 63 -f ncopy.ys > ldriver.ys
../misc/yas ldriver.ys
```
首先测试一个简单的 Y86-64 程序：
```
/psim -t ../y86-code/asumi.yo -v 1
48 instructions executed
Status = HLT
Condition Codes: Z=1 S=0 O=0
Changed Register State:
%rax:   0x0000000000000000      0x0000abcdabcdabcd
%rsp:   0x0000000000000000      0x0000000000000100
%rdi:   0x0000000000000000      0x0000000000000038
%r10:   0x0000000000000000      0x0000a000a000a000
Changed Memory State:
0x00f0: 0x0000000000000000      0x0000000000000055
0x00f8: 0x0000000000000000      0x0000000000000013
ISA Check Succeeds
CPI: 44 cycles/32 instructions = 1.38
```
进行基准测试：
```
cd ../y86-code; make testpsim
Makefile:42: warning: ignoring prerequisites on suffix rule definition
Makefile:45: warning: ignoring prerequisites on suffix rule definition
Makefile:48: warning: ignoring prerequisites on suffix rule definition
Makefile:51: warning: ignoring prerequisites on suffix rule definition
../pipe/psim -t asum.yo > asum.pipe
../pipe/psim -t asumr.yo > asumr.pipe
../pipe/psim -t cjr.yo > cjr.pipe
../pipe/psim -t j-cc.yo > j-cc.pipe
../pipe/psim -t poptest.yo > poptest.pipe
../pipe/psim -t pushquestion.yo > pushquestion.pipe
../pipe/psim -t pushtest.yo > pushtest.pipe
../pipe/psim -t prog1.yo > prog1.pipe
../pipe/psim -t prog2.yo > prog2.pipe
../pipe/psim -t prog3.yo > prog3.pipe
../pipe/psim -t prog4.yo > prog4.pipe
../pipe/psim -t prog5.yo > prog5.pipe
../pipe/psim -t prog6.yo > prog6.pipe
../pipe/psim -t prog7.yo > prog7.pipe
../pipe/psim -t prog8.yo > prog8.pipe
../pipe/psim -t ret-hazard.yo > ret-hazard.pipe
grep "ISA Check" *.pipe
asum.pipe:ISA Check Succeeds
asumr.pipe:ISA Check Succeeds
cjr.pipe:ISA Check Succeeds
j-cc.pipe:ISA Check Succeeds
poptest.pipe:ISA Check Succeeds
prog1.pipe:ISA Check Succeeds
prog2.pipe:ISA Check Succeeds
prog3.pipe:ISA Check Succeeds
prog4.pipe:ISA Check Succeeds
prog5.pipe:ISA Check Succeeds
prog6.pipe:ISA Check Succeeds
prog7.pipe:ISA Check Succeeds
prog8.pipe:ISA Check Succeeds
pushquestion.pipe:ISA Check Succeeds
pushtest.pipe:ISA Check Succeeds
ret-hazard.pipe:ISA Check Succeeds
rm asum.pipe asumr.pipe cjr.pipe j-cc.pipe poptest.pipe pushquestion.pipe pushtest.pipe prog1.pipe prog2.pipe prog3.pipe prog4.pipe prog5.pipe prog6.pipe prog7.pipe prog8.pipe ret-hazard.pipe
```
进行回归测试：
```
make SIM=../pipe/psim
./optest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 49 ISA Checks Succeed
./jtest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 64 ISA Checks Succeed
./ctest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 22 ISA Checks Succeed
./htest.pl -s ../pipe/psim 
Simulating with ../pipe/psim
  All 600 ISA Checks Succeed
```
进行回归测试：
```
make SIM=../pipe/psim TFLAGS=-i
./optest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 58 ISA Checks Succeed
./jtest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 96 ISA Checks Succeed
./ctest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 22 ISA Checks Succeed
./htest.pl -s ../pipe/psim -i
Simulating with ../pipe/psim
  All 756 ISA Checks Succeed
```
上述所有测试均通过。


#### ncopy.ys
ncopy.ys 是 ncopy.c 的 Y86-64 实现，
```c
/* $begin ncopy */
/*
 * ncopy - copy src to dst, returning number of positive ints
 * contained in src array.
 */
word_t ncopy(word_t *src, word_t *dst, word_t len)
{
    word_t count = 0;
    word_t val;

    while (len > 0) {
        val = *src++;
        *dst++ = val;
        if (val > 0)
            count++;
        len--;
    }
    return count;
}
/* $end ncopy */
```
基准版本的 ncopy.ys 如下：
```
#/* $begin ncopy-ys */
##################################################################
# ncopy.ys - Copy a src block of len words to dst.
# Return the number of positive words (>0) contained in src.
#
# Include your name and ID here.
#
# Describe how and why you modified the baseline code.
#
##################################################################
# Do not modify this portion
# Function prologue.
# %rdi = src, %rsi = dst, %rdx = len
ncopy:

##################################################################
# You can modify this portion
	# Loop header
	xorq %rax,%rax		# count = 0;
	andq %rdx,%rdx		# len <= 0?
	jle Done			# if so, goto Done:

Loop:	
	mrmovq (%rdi), %r10	# read val from src...
	rmmovq %r10, (%rsi)	# ...and store it to dst
	andq %r10, %r10		# val <= 0?
	jle Npos			# if so, goto Npos:
	irmovq $1, %r10
	addq %r10, %rax		# count++
Npos:	
    irmovq $1, %r10
	subq %r10, %rdx		# len--
	irmovq $8, %r10
	addq %r10, %rdi		# src++
	addq %r10, %rsi		# dst++
	andq %rdx,%rdx		# len > 0?
	jg Loop				# if so, goto Loop:
##################################################################
# Do not modify the following section of code
# Function epilogue.
Done:
	ret
##################################################################
# Keep the following label at the end of your function
End:
#/* $end ncopy-ys */

```
我们可以采取如下措施对 ncopy.ys 进行优化：
1. 指令重排
2. 使用一条指令代替多条指令
3. 删除无关指令
4. 添加其他指令
5. 循环展开

我们将 iaddq 指令替代 ncopy.ys 中的多条指令：
```
##################################################################
# Do not modify this portion
# Function prologue.
# %rdi = src, %rsi = dst, %rdx = len
ncopy:

##################################################################
# You can modify this portion
	# Loop header
	xorq %rax,%rax		# count = 0;
	andq %rdx,%rdx		# len <= 0?
	jle Done			# if so, goto Done:

Loop:	
	mrmovq (%rdi), %r10	# read val from src...
	rmmovq %r10, (%rsi)	# ...and store it to dst
	andq %r10, %r10		# val <= 0?
	jle Npos			# if so, goto Npos:
	iaddq $1, %rax	# count++
Npos:	
	iaddq $-1, %rdx		# len--
	iaddq $8, %rdi		# src++
	iaddq $8, %rsi		# dst++
 	andq %rdx,%rdx		# len > 0?
	jg Loop				# if so, goto Loop:
##################################################################
# Do not modify the following section of code
# Function epilogue.
Done:
	ret
##################################################################
```
然后，我们进行指令重排优化，发现如下两条指令出现取数/使用型数据冒险：
```
mrmovq (%rdi), %r10	# read val from src...
rmmovq %r10, (%rsi)	# ...and store it to dst
```
取数/使用型数据冒险需要**暂停**一个时钟周期并且使用数据转发技术才可以解决，因此插入一条无关指令 `iaddq $8, %rdi`，消除该取数/使用型冒险：
```
##################################################################
# You can modify this portion
	# Loop header
	xorq %rax,%rax		# count = 0;
	andq %rdx,%rdx		# len <= 0?
	jle Done			# if so, goto Done:

Loop:	
	mrmovq (%rdi), %r10	# read val from src...
	iaddq $8, %rdi		# src++
	rmmovq %r10, (%rsi)	# ...and store it to dst
	andq %r10, %r10		# val <= 0?
	jle Npos			# if so, goto Npos:
	iaddq $1, %rax	# count++
Npos:
	iaddq $8, %rsi		# dst++
	iaddq $-1, %rdx		# len--
	jg Loop				# if so, goto Loop:
##################################################################
```

