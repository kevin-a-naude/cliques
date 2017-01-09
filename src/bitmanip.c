
#include "bitmanip.h"

/* Inverse lookup for De Bruijn sequence given by 0x043147259A7ABB7E */
int InverseDeBruijnSubsequenceTable[64] =
{
    63, 0, 1, 6, 2, 12, 7, 18,
    3, 24, 13, 27, 8, 33, 19, 39,
    4, 16, 25, 37, 14, 45, 28, 47,
    9, 30, 34, 53, 20, 49, 40, 56,
    62, 5, 11, 17, 23, 26, 32, 38,
    15, 36, 44, 46, 29, 52, 48, 55,
    61, 10, 22, 31, 35, 43, 51, 54,
    60, 21, 42, 50, 59, 41, 58, 57
};
