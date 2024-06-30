unsigned srl(unsigned x, int k)
{
    /* Perform shift arithmetically */
    unsigned xsra = (int)x >> k;
    int w = 8 * sizeof(int);
    int shift_val = (w - k) & (w - 1); /* 位移量 */
    int mask1 = (1 << shift_val) - 1;
    int mask2 = (!mask1 << (w - 1)) >> (w - 1);
    int mask = (mask2 & ~0) | (~mask2 & mask1);

    return xsra & mask;
}