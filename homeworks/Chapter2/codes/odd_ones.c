/* Return 1 when x contains an odd number of 1s; 0 otherwise.
   Assume w=32 */
int odd_ones(unsigned x)
{
    unsigned left = x >> 16; /* Get Higher 16 bits  */

    x ^= left;
    left = x >> 8; /* Get Higher 8 bits */

    x ^= left;
    left = x >> 4; /* Get Higher 4 bits */

    x ^= left;
    left = x >> 2; /* Get Higher 2 bits */

    x ^= left;
    left = x >> 1; /* Get Higher 1 bit */

    return (x ^ left) & 0x1;
}