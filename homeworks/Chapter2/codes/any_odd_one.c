int any_odd_one(unsigned x)
{
    /* mask=0xAAAAAAAA */
    unsigned mask = (0xAA << 24) | (0xAA << 16) | (0xAA << 8) | 0xAA;
    return !((x & mask) ^ mask);
}