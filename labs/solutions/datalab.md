# Datalab

> Datalab requires us to implement simple logical, two's complement, and floating point functions, but using a highly restricted subset of C. For example, they might be asked to compute the absolute value of a number using only bit-level operations and straightline code. This lab helps us understand the bit-level representations of C data types and the bit-level behavior of the operations on data.

注：在 [CSAPP 官网](https://csapp.cs.cmu.edu/3e/labs.html) 中，可以下载所有不含具体答案的 CSAPP 实验。

本文档包含两个部分：
1. Introduction 部分介绍了 Datalab 的细节，包括编码风格、编码规则等内容，该部分内容由 Datalab 的实验手册中直接复制。
2. Solutions 部分具体分析了每个函数的实现思路，并给出具体的实现代码。

## Introduction
The following table lists the puzzles in rought order of difficulty ***from easiest to hardest***. The "Rating" field gives the difficulty rating (the number of *points*) for the puzzle, and the "Max ops" field gives the ***maximum*** number of operators we are allowed to use to implement each function.
| Name | Description | Rating | Max ops |
| ---- | ----------- | ------ | ------- |
| bitXor(x, y) | x ^ y using only & and ~. | 1 | 14 |
| tmin() | Smallest two’s complement integer | 1 | 4 |
| isTmax(x) | True only if x x is largest two’s comp. integer. | 1 | 10 |
| allOddBits(x) | True only if all odd-numbered bits in x set to 1. | 2 | 12 |
| negate(x) | Return -x with using - operator. | 2 | 5 |
| isAsciDigit(x) | True if 0x30 <= x <= 0x39. | 3 | 15 |
| conditional | Same as x ? y : z | 3 | 16 |
| isLessOrEqual(x, y) | True if x <= y, false otherwise | 3 | 24 |
| logicalNeg(x) | Compute !x without using ! operator. | 4 | 12 |
| howManyBits(x) | Min. no. of bits to represent x in two’s comp. | 4 | 90 |
| floatScale2(uf) | Return bit-level equiv. of 2*f for f.p. arg. f. | 4 | 30 |
| floatFloat2Int(uf) | Return bit-level equiv. of (int)f for f.p. arg. f. | 4 | 30 |
| floatPower2(x) | Return bit-level equiv. of 2.0ˆx for integer x. | 4 | 30 |

For the floating-point puzzles, we will implement some common ***single-precision*** floating-point operations. For these puzzles, we are allowed to use standard control structures (conditionals, loops), and we may use both `int` and `unsigned` data types, including arbitrary unsigned and integer constants. 

we may not use any unions, structs, or arrays. Most significantly, we may not use any floating point data types, operations, or constants. Instead, any floating-point operand will be passed to the function as having type `unsigned`, and any returned floating-point value will be of type `unsigned`. Our code should perform the bit manipulations that implement the specified floating point operations.


### Coding rules

#### Integer Coding rules
Replace the "return" statement in each function with one or more lines of C code that implements the function. Our code must conform to the following style:
```c
int Funct(arg1, arg2, ...) {
    /* brief description of how our implementation works */
    int var1 = Expr1;
    ...
    int varM = ExprM;

    varJ = ExprJ;
    ...
    varN = ExprN;
    return ExprR;
}
```
Each "Expr" is an expression using ***ONLY*** the following:
1. Integer constants 0 through 255 (0xFF), inclusive. We are not allowed to use big constants such as 0xffffffff.
2. Function arguments and local variables (no global variables).
3. Unary integer operations ! ~
4. Binary integer operations & ^ | + << >>

Some of the problems restrict the set of allowed operators even further.Each "Expr" may consist of multiple operators. We are not restricted to one operator per line.

We are expressly ***forbidden*** to:
1. Use any control constructs such as if, do, while, for, switch, etc.
2. Define or use any macros.
3. Define any additional functions in this file.
4. Call any functions.
5. Use any other operations, such as &&, ||, -, or ?:
6. Use any form of casting.
7. Use any data type other than int.  This implies that we cannot use arrays, structs, or unions.

We may assume that our machine:
1. Uses tow's complement, 32-bit representations of integers.
2. Performs right shifts arithmetically.
3. Has unpredictable behavior when shifting if the shift amount is less than 0 or greater than 31.

***EXAMPLES OF ACCEPTABLE CODING STYLE***:
```c
/*
* pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
*/
int pow2plus1(int x) {
    /* exploit ability of shifts to compute powers of 2 */
    return (1 << x) + 1;
}

/*
* pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
*/
int pow2plus4(int x) {
    /* exploit ability of shifts to compute powers of 2 */
    int result = (1 << x);
    result += 4;
    return result;
}
```

#### Floating point Coding rules
For the problems that require we to implement floating-point operations, the coding rules are ***less strict***.  We are allowed to use looping and
conditional control.  We are allowed to use both ints and unsigneds. We can use arbitrary integer and unsigned constants. We can use any arithmetic, logical, or comparison operations on int or unsigned data.

We are expressly ***forbidden*** to:
1. Define or use any macros.
2. Define any additional functions in this file.
3. Call any functions.
4. Use any form of casting.
5. Use any data type other than int or unsigned.  This means that we
    cannot use arrays, structs, or unions.
6. Use any floating point data types, operations, or constants.


***NOTES***:
1. Use the dlc (data lab checker) compiler (described in the handout) to check the legality of our solutions.
2. Each function has a maximum number of operations (integer, logical, or comparison) that we are allowed to use for Our implementation of the function.  The max operator count is checked by dlc. Note that assignment ('=') is not counted; we may use as many of these as we want without penalty.
3. Use the btest test harness to check our functions for correctness.
4. Use the BDD checker to formally verify our functions
5. The maximum number of ops for each function is given in the header comment for each function. If there are any inconsistencies  between the maximum ops in the writeup and in this file, consider this file the authoritative source.

## Solutions

### bitXor
bitXor 要求我们通过 `~` 和 `&` 运算符实现 ***XOR*** 运算符，因为 `x ^ y` 等价于 `(~x & y) | (~y & x)`，然而题目中不可以使用 `|` 运算符，可利用***德摩根律***（De Morgan Law）将 `|` 运算符仅用 `~` 和 `&` 运算符表示，因此 `x ^ y` 等价于 `~(~(~x & y) & ~(~y & x))`。
```c
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  return ~(~(~x & y) & ~(~y & x));
}
```

### tmin

最小的二进制补码整数的值（TMin32）为 `0x80000000`，可以通过左移运算 `1 << 31` 生成 TMin32。
```c
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
```

### isTmax
最大的二进制补码数（TMax32）的值为 `0x7FFFFFFF`，实际上，我们可以通过表达式 TMax32 + 1 得到 TMin32（正溢出），因为共有以下两种场景会使得 `x+x` 的结果为 0：
1. x 等于 0
2. x 等于 TMin32

因此

### negate
可以通过 `~` 和 `+` 运算符实现相反数运算，根据位级整数的性质，`-x` 等价于 `~x+1`。
```c
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
```

### conditional

```c
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int mask = x >> 31;
  return (mask & y) | (!mask & z);
}
```