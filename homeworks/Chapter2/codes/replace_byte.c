unsigned replace_byte(unsigned x, int i, unsigned char b)
{
    unsigned shift_val = i * 8; /* 位移量 */
    unsigned mask = ~(0xff << shift_val); /* 掩码 */
    return (x & mask) | (b << shift_val);
}