# Bomblab
> A "binary bomb" is a program provided to students as an ***object code file***. When run, it prompts the user to type in 6 different strings. If any of these is incorrect, the bomb "explodes," printing an error message and logging the event on a grading server. Students must "defuse" their own unique bomb by ***disassembling*** and ***reverse engineering*** the program to determine what the 6 strings should be. The lab teaches students to understand assembly language, and also forces them to learn how to use a debugger. It's also great fun. A ***legendary lab*** among the CMU undergrads.

Bomblab 是 CSAPP 这门课程口碑最好的实验，并且也是最有趣的实验。Bomblab 主要给出两个文件：二进制 bomb 文件、bomb.c 源文件。有很多细节需要我们通过逆向工程分析。

注：在 [CSAPP 官网](https://csapp.cs.cmu.edu/3e/labs.html) 中，可以下载所有不含具体答案的 CSAPP 实验。

本文档包含两个部分：

1. Introduction 部分介绍了 Bomblab，该部分内容由 Bomblab 的实验手册中直接复制。
2. Solutions 部分具体分析了每个阶段的拆弹思路，并给出具体的分析过程以及每个阶段对应的 C 代码。

## Introduction
The nefarious *Dr. Evil* has planted a slew of “binary bombs” on our class machines. A binary bomb is a program that consists of a sequence of phases. Each phase expects you to type a particular string on **stdin**. If you type the correct string, then the phase is *defused* and the bomb proceeds to the next phase. Otherwise, the bomb *explodes* by printing "BOOM!!!" and then terminating. The bomb is defused when every phase has been defused.

There are too many bombs for us to deal with, so we are giving each student a bomb to defuse. Your
mission, which you have no choice but to accept, is to defuse your bomb before the due date. Good luck,
and welcome to the bomb squad!


## Solutions⭐
由于 Bomblab 的逆向工程需要使用 GDB，在 resource/gdbnotes-x86-64.pdf 给出了常用的 GDB 指令作为参考。

首先，通过以下指令进入 GDB 调试：
```bash
gdb bomb
```

### Phase 1
运行反汇编指令，获取 Phase 1 对应的汇编代码：
```
(gdb) dias phase_1
Dump of assembler code for function phase_1:
   0x0000000000400ee0 <+0>:     sub    $0x8,%rsp
   0x0000000000400ee4 <+4>:     mov    $0x402400,%esi
   0x0000000000400ee9 <+9>:     call   0x401338 <strings_not_equal>
   0x0000000000400eee <+14>:    test   %eax,%eax
   0x0000000000400ef0 <+16>:    je     0x400ef7 <phase_1+23>
   0x0000000000400ef2 <+18>:    call   0x40143a <explode_bomb>
   0x0000000000400ef7 <+23>:    add    $0x8,%rsp
   0x0000000000400efb <+27>:    ret    
End of assembler dump.
```
**分析**：Phase 1 首先调用了一个 `strings_not_equal` 函数，若该返回值为 0，则该阶段拆弹成功。通过函数的名称可以猜测到函数的功能，当输入的字符串 s 与内存地址 *0x402400* 开始的字符串相等时，返回 0，否则返回 1。

运行以下指令，查看内存地址 *0x402400* 开始的字符串：
```
(gdb) print (char *)0x402400
$1 = 0x402400 "Border relations with Canada have never been better."
```
因此 **Phase 1 的答案为**：`"Border relations with Canada have never been better."`。测试通过：
```
(gdb) run
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Border relations with Canada have never been better.
Phase 1 defused. How about the next one?
```
Phase 1 对应的 C 语言代码为：
```c
void phase_1(char *s)
{
    char *target = "Border relations with Canada have never been better.";
    if (string_not_equal(s, target))
        explode_bomb();
}
```

### Phase 2
运行反汇编指令，获取 Phase 2 对应的汇编代码：
```
(gdb) disas phase_2
Dump of assembler code for function phase_2:
   0x0000000000400efc <+0>:     push   %rbp
   0x0000000000400efd <+1>:     push   %rbx
   0x0000000000400efe <+2>:     sub    $0x28,%rsp
   0x0000000000400f02 <+6>:     mov    %rsp,%rsi
   0x0000000000400f05 <+9>:     call   0x40145c <read_six_numbers>
   0x0000000000400f0a <+14>:    cmpl   $0x1,(%rsp)
   0x0000000000400f0e <+18>:    je     0x400f30 <phase_2+52>
   0x0000000000400f10 <+20>:    call   0x40143a <explode_bomb>
   0x0000000000400f15 <+25>:    jmp    0x400f30 <phase_2+52>
   0x0000000000400f17 <+27>:    mov    -0x4(%rbx),%eax
   0x0000000000400f1a <+30>:    add    %eax,%eax
   0x0000000000400f1c <+32>:    cmp    %eax,(%rbx)
   0x0000000000400f1e <+34>:    je     0x400f25 <phase_2+41>
   0x0000000000400f20 <+36>:    call   0x40143a <explode_bomb>
   0x0000000000400f25 <+41>:    add    $0x4,%rbx
   0x0000000000400f29 <+45>:    cmp    %rbp,%rbx
   0x0000000000400f2c <+48>:    jne    0x400f17 <phase_2+27>
   0x0000000000400f2e <+50>:    jmp    0x400f3c <phase_2+64>
   0x0000000000400f30 <+52>:    lea    0x4(%rsp),%rbx
   0x0000000000400f35 <+57>:    lea    0x18(%rsp),%rbp
   0x0000000000400f3a <+62>:    jmp    0x400f17 <phase_2+27>
   0x0000000000400f3c <+64>:    add    $0x28,%rsp
   0x0000000000400f40 <+68>:    pop    %rbx
   0x0000000000400f41 <+69>:    pop    %rbp
   0x0000000000400f42 <+70>:    ret    
End of assembler dump.
```

**分析**：首先，Phase 2 在栈上为局部变量（数组）分配空间，并调用 `read_six_numbers` 读入 6 个整数。若第 1 个整数不等于 1，炸弹将会引爆。
```
call   0x40145c <read_six_numbers>
cmpl   $0x1,(%rsp)
je     0x400f30 <phase_2+52>
call   0x40143a <explode_bomb>
```
因此我们得出**第 1 个整数为 1**。

当输入的第 1 个整数为 1 时，接着会执行下列指令，实际上为**循环的初始化**：
```
lea    0x4(%rsp),%rbx
lea    0x18(%rsp),%rbp
jmp    0x400f17 <phase_2+27>
```
初始化后，寄存器 %rbx、%rbp 分别指向第 2、第 6 个整数的内存地址。

循环对应的汇编代码如下：
```
mov    -0x4(%rbx),%eax
add    %eax,%eax
cmp    %eax,(%rbx)
je     0x400f25 <phase_2+41>
call   0x40143a <explode_bomb>
add    $0x4,%rbx
cmp    %rbp,%rbx
jne    0x400f17 <phase_2+27>
jmp    0x400f3c <phase_2+64>
```
这个循环用于**检测第 n 个整数是否为第 n - 1 个整数的两倍**，若不是，则炸弹将被引爆，由于我们已经确定第 1 个整数为 1，因此后续的 5 个整数分别为：2、4、8、16、32。

综上所述，Phase 2 的答案为：`1 2 4 8 16 32`。测试通过：
```
Phase 1 defused. How about the next one?
1 2 4 8 16 32
That's number 2.  Keep going!
```
Phase 2 对应的 C 语言代码为：
```c
#define SIZE 6
void phase_2()
{
    int i;
    int nums[SIZE];
    read_six_numbers(nums);
    
    if (nums[0] != 0)
        explode_bomb();
    
    for (i = 1; i < SIZE; i++)
        if (nums[i] != 2 * nums[i - 1])
            explode_bomb();
}
```


### Phase 3
运行反汇编指令，获取 Phase 3 对应的汇编代码：
```
(gdb) disas phase_3
Dump of assembler code for function phase_3:
   0x0000000000400f43 <+0>:     sub    $0x18,%rsp
   0x0000000000400f47 <+4>:     lea    0xc(%rsp),%rcx
   0x0000000000400f4c <+9>:     lea    0x8(%rsp),%rdx
   0x0000000000400f51 <+14>:    mov    $0x4025cf,%esi
   0x0000000000400f56 <+19>:    mov    $0x0,%eax
   0x0000000000400f5b <+24>:    call   0x400bf0 <__isoc99_sscanf@plt>
   0x0000000000400f60 <+29>:    cmp    $0x1,%eax
   0x0000000000400f63 <+32>:    jg     0x400f6a <phase_3+39>
   0x0000000000400f65 <+34>:    call   0x40143a <explode_bomb>
   0x0000000000400f6a <+39>:    cmpl   $0x7,0x8(%rsp)
   0x0000000000400f6f <+44>:    ja     0x400fad <phase_3+106>
   0x0000000000400f71 <+46>:    mov    0x8(%rsp),%eax
   0x0000000000400f75 <+50>:    jmp    *0x402470(,%rax,8)
   0x0000000000400f7c <+57>:    mov    $0xcf,%eax
   0x0000000000400f81 <+62>:    jmp    0x400fbe <phase_3+123>
   0x0000000000400f83 <+64>:    mov    $0x2c3,%eax
   0x0000000000400f88 <+69>:    jmp    0x400fbe <phase_3+123>
   0x0000000000400f8a <+71>:    mov    $0x100,%eax
   0x0000000000400f8f <+76>:    jmp    0x400fbe <phase_3+123>
   0x0000000000400f91 <+78>:    mov    $0x185,%eax
   0x0000000000400f96 <+83>:    jmp    0x400fbe <phase_3+123>
   0x0000000000400f98 <+85>:    mov    $0xce,%eax
   0x0000000000400f9d <+90>:    jmp    0x400fbe <phase_3+123>
   0x0000000000400f9f <+92>:    mov    $0x2aa,%eax
   0x0000000000400fa4 <+97>:    jmp    0x400fbe <phase_3+123>
   0x0000000000400fa6 <+99>:    mov    $0x147,%eax
   0x0000000000400fab <+104>:   jmp    0x400fbe <phase_3+123>
   0x0000000000400fad <+106>:   call   0x40143a <explode_bomb>
   0x0000000000400fb2 <+111>:   mov    $0x0,%eax
   0x0000000000400fb7 <+116>:   jmp    0x400fbe <phase_3+123>
   0x0000000000400fb9 <+118>:   mov    $0x137,%eax
   0x0000000000400fbe <+123>:   cmp    0xc(%rsp),%eax
   0x0000000000400fc2 <+127>:   je     0x400fc9 <phase_3+134>
   0x0000000000400fc4 <+129>:   call   0x40143a <explode_bomb>
   0x0000000000400fc9 <+134>:   add    $0x18,%rsp
   0x0000000000400fcd <+138>:   ret    
End of assembler dump.
```
**分析**：首先，Phase 3 在栈上分配局部变量，随后调用库函数进行输入。
```
sub    $0x18,%rsp
lea    0xc(%rsp),%rcx
lea    0x8(%rsp),%rdx
mov    $0x4025cf,%esi
mov    $0x0,%eax
call   0x400bf0 <__isoc99_sscanf@plt>
cmp    $0x1,%eax
jg     0x400f6a <phase_3+39>
call   0x40143a <explode_bomb>
```
运行以下指令，查看内存地址 *0x4025cf* 的字符串内容：
```
(gdb) print (char *)0x4025cf
$2 = 0x4025cf "%d %d"
```
其实就是 C 语言的 scanf 函数的输入格式串，实际上需要我们输入两个整数 x、y，**若输入的整数个数小于 2，炸弹将会被引爆**。

若输入的整数个数大于或等于 2，则会执行下列指令，**若第 1 个整数大于 7，则炸弹将会引爆**。
```
cmpl   $0x7,0x8(%rsp)
ja     0x400fad <phase_3+106>
...
call   0x40143a <explode_bomb> # phase_3+106
```
若炸弹未被引爆，则执行下列的指令，明显是一个 switch 语句，根据输入的第一个整数作为根据，进行跳转，其中**跳转表**（Jump Table）的存放于 *0x402470*，根据下列代码，不难看出跳转表中共有 8 个 case。
```
mov    0x8(%rsp),%eax
jmp    *0x402470(,%rax,8)
mov    $0xcf,%eax
jmp    0x400fbe <phase_3+123>
mov    $0x2c3,%eax
jmp    0x400fbe <phase_3+123>
mov    $0x100,%eax
jmp    0x400fbe <phase_3+123>
mov    $0x185,%eax
jmp    0x400fbe <phase_3+123>
mov    $0xce,%eax
jmp    0x400fbe <phase_3+123>
mov    $0x2aa,%eax
jmp    0x400fbe <phase_3+123>
mov    $0x147,%eax
jmp    0x400fbe <phase_3+123>
call   0x40143a <explode_bomb>
mov    $0x0,%eax
jmp    0x400fbe <phase_3+123>
mov    $0x137,%eax
cmp    0xc(%rsp),%eax
je     0x400fc9 <phase_3+134>
call   0x40143a <explode_bomb>
add    $0x18,%rsp
ret
```
运行以下指令，查看跳转表的 8 个地址：
```
(gdb) x/8g 0x402470
0x402470:       0x0000000000400f7c      0x0000000000400fb9
0x402480:       0x0000000000400f83      0x0000000000400f8a
0x402490:       0x0000000000400f91      0x0000000000400f98
0x4024a0:       0x0000000000400f9f      0x0000000000400fa6
```
分析 switch 语句可知，其逻辑就是根据第一个输入的整数 x(0 <= x <= 7) 来匹配第二个整数，两个整数具有对应关系，结合跳转表以及switch 语句对应的汇编代码可得 Phase 3 的答案如下表所示（**任意一行均可**）：
| x | y |
| --- | --- |
| 0 | 207 |
| 1 | 311 |
| 2 | 707 |
| 3 | 256 |
| 4 | 389 |
| 5 | 206 |
| 6 | 682 |
| 7 | 327 |

注：上述答案均通过 Phase 3 的测试。

Phase 3 对应的 C 语言代码如下：
```c
void phase_3()
{
    int x, y, n;
    if (scanf("%d %d", &x, &y) < 2)
        explode_bomb();
    
    switch (x)
    {
        case 0:
            if (y != 207) explode_bomb();
            break;
        case 1:
            if (y != 311) explode_bomb();
            break;
        case 2:
            if (y != 707) explode_bomb();
            break;
        case 3:
            if (y != 256) explode_bomb();
            break;
        case 4:
            if (y != 389) explode_bomb();
            break;
        case 5:
            if (y != 206) explode_bomb();
            break;
        case 6:
            if (y != 682) explode_bomb();
            break;
        case 7:
            if (y != 327) explode_bomb();
            break;
        default:
            explode_bomb();
    }
}
```

### Phase 4

### Phase 5

### Phase 6

### Secret Pahse

