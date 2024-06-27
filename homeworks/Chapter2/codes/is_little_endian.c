#include <stdio.h>

typedef unsigned char *byte_pointer;

int is_little_endian()
{
    unsigned x = 1;
    byte_pointer ptr = (byte_pointer)&x;

    return *ptr;
}