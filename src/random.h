
#pragma once

#ifndef RANDOM_H
#define RANDOM_H 1

/* This is the JKISS PRNG.  It is a simple, robust, long-period random number. */
/* Replace with something better, if you like. */

#include <stdint.h>
#include <time.h>

typedef struct
{
    uint32_t x, y, z, c;
} prng_t;

static inline uint32_t prng_uint32(prng_t* self)
{
    uint64_t t;
    uint32_t x = self->x;
    uint32_t y = self->y;
    uint32_t z = self->z;
    uint32_t c = self->c;
    x = 314527869 * x + 1234567;
    y ^= y << 5; y ^= y >> 7; y ^= y << 22;
    t = UINT64_C(0xFFFA2849) * z + c; c = (uint32_t)(t >> 32); z = (uint32_t)t;
    self->x = x;
    self->y = y;
    self->z = z;
    self->c = c;
    return x + y + z;
}

static inline uint32_t prng_uint32_capped(prng_t* self, uint32_t cap)
{
    if (cap <= 1) return 0;

    uint32_t num = prng_uint32(self);

    if ((cap & (uint32_t)(-(int32_t)cap)) == cap)
    {
        /* cap is a power of 2 */
        return num & (cap - 1);
    }
    else
    {
        uint32_t divisor = ((uint32_t)0xFFFFFFFFU) / cap;
        uint32_t threshold = cap * divisor;
        while (num >= threshold)
        {
            num = prng_uint32(self);
        }
        return num / divisor;
    }
}

static inline double prng_double(prng_t* self)
{
    double r = prng_uint32(self) / 4294967296.0;
    r += (prng_uint32(self) & 0x1FFFFFU) / (4294967296.0 * 2097152.0);
    return r;
}


/* I've spiced up the initialisation a bit. */

static inline void prng_seed(prng_t* self, uint32_t seed)
{
    uint32_t a = seed & 0x3FF;
    uint32_t b = ((seed >> 10) & 0x1F) ^ (a << 7);
    uint32_t c = ((seed >> 15) & 0x1F) ^ (a << 5);
    uint32_t d = ((seed >> 20) & 0x1F) ^ (a << 3);
    uint32_t e = ((seed >> 25) & 0x7F) ^ (a << 1);

    uint32_t spin = 1024 + a;
    self->x = 123456789 ^ b;
    self->y = 987654321 ^ c;
    self->z = 43219876 ^ d;
    self->c = 6543217 ^ e;
    while (spin-- > 0)
    {
        prng_uint32(self);
    }
}

static inline void prng_init(prng_t* self)
{
    prng_seed(self, (uint32_t)time(NULL));
}

#endif
