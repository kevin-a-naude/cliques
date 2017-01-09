
#pragma once

#ifndef BITMANIP_H
#define BITMANIP_H 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

extern int InverseDeBruijnSubsequenceTable[64];

static inline int bit_to_index(uint64_t bit)
{
    return InverseDeBruijnSubsequenceTable[(size_t)(((uint64_t)bit * UINT64_C(0x043147259A7ABB7E)) >> 58)];
}

static inline uint64_t single_bit(int index)
{
    return ((uint64_t)1) << index;
}

static inline uint64_t all_higher_bits(uint64_t bit)
{
    return (~(bit - 1)) ^ bit;
}

static inline uint64_t lowest_bit(uint64_t bits)
{
    return (bits & (uint64_t)(-(int64_t)bits));
}

static inline int lowest_bit_index(uint64_t bits)
{
    return bit_to_index(lowest_bit(bits));
}

static inline int highest_bit_index(uint64_t bits)
{
    int position;
    int shift;

    position = (bits > 0xFFFFFFFF) << 5; bits >>= position;
    shift = (bits > 0xFFFF) << 4; bits >>= shift; position |= shift;
    shift = (bits > 0xFF) << 3; bits >>= shift; position |= shift;
    shift = (bits > 0xF) << 2; bits >>= shift; position |= shift;
    shift = (bits > 0x3) << 1; bits >>= shift; position |= shift;
    position |= (bits >> 1);

    return position;
}

static inline uint64_t highest_bit(uint64_t bits)
{
    return single_bit(highest_bit_index(bits));
}

static inline int count_bits(uint64_t bits)
{
    int sum = 0;
    while (bits != 0)
    {
        uint64_t bit = lowest_bit(bits);

        sum++;

        bits ^= bit;
    }
    return sum;
}

#endif
