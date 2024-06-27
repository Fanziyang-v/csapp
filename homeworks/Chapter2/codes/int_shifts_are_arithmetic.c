int int_shifts_are_arithmetic()
{
    unsigned shift_val = (sizeof(int) - 1) << 3;
    int x = 0xff << shift_val;
    int y = x >> shift_val;

    return !(x ^ y);
}