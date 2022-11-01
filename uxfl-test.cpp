#include <stdio.h>
#include <stdint.h>
#include <stdexcept>
#include <cmath>
static int64_t const minMantissa = 1000000000000000ull;
    static int64_t const maxMantissa = 9999999999999999ull;
    static int32_t const minExponent = -96;
    static int32_t const maxExponent = 80;
    inline int32_t get_exponent(int64_t float1)
    {
        if (float1 < 0)
            throw std::runtime_error("INVALID_FLOAT");
        if (float1 == 0)
            return 0;
        if (float1 < 0)
            throw std::runtime_error("INVALID_FLOAT");
        uint64_t float_in = (uint64_t)float1;
        float_in >>= 54U;
        float_in &= 0xFFU;
        return ((int32_t)float_in) - 97;
    }

    inline uint64_t get_mantissa(int64_t float1)
    {
        if (float1 < 0)
            throw std::runtime_error("INVALID_FLOAT");
        if (float1 == 0)
            return 0;
        if (float1 < 0)
            throw std::runtime_error("INVALID_FLOAT");
        float1 -= ((((uint64_t)float1) >> 54U) << 54U);
        return float1;
    }

    inline bool is_negative(int64_t float1)
    {
        return (float1 >> 62U) != 0;
    }

    inline int64_t invert_sign(int64_t float1)
    {
        return float1 ^ (1ULL<<62U);
    }

    inline int64_t set_sign(int64_t float1, bool set_negative)
    {
        bool neg = is_negative(float1);
        if ((neg && set_negative) || (!neg && !set_negative))
            return float1;

        return invert_sign(float1);
    }

    inline int64_t set_mantissa(int64_t float1, uint64_t mantissa)
    {
        if (mantissa > maxMantissa)
            throw std::runtime_error("MANTISSA_OVERSIZED");
        return float1 - get_mantissa(float1) + mantissa;
    }

    inline int64_t set_exponent(int64_t float1, int32_t exponent)
    {
        if (exponent > maxExponent)
            throw std::runtime_error("EXPONENT_OVERSIZED");
        if (exponent < minExponent)
            throw std::runtime_error("EXPONENT_UNDERSIZED");

        uint64_t exp = (exponent + 97);
        exp <<= 54U;
        float1 &= ~(0xFFLL<<54);
        float1 += (int64_t)exp;
        return float1;
    }
    inline int64_t make_float(int64_t mantissa, int32_t exponent)
    {
        if (mantissa == 0)
            return 0;
        if (mantissa > maxMantissa)
            throw std::runtime_error("MANTISSA_OVERSIZED");
        if (exponent > maxExponent)
            throw std::runtime_error("EXPONENT_OVERSIZED");
        if (exponent < minExponent)
            throw std::runtime_error("EXPONENT_UNDERSIZED");
        bool neg = mantissa < 0;
        if (neg)
            mantissa *= -1LL;
        int64_t out =  0;
        out = set_mantissa(out, mantissa);
        out = set_exponent(out, exponent);
        out = set_sign(out, neg);
        return out;
    }

int64_t unpack_xfl(int64_t packed)
{
    if (is_negative(packed))
        return ~packed;
    return packed;
}

int64_t pack_xfl(int64_t unpacked)
{
    if (unpacked < 0)
        return ~unpacked;
    return unpacked;
}

int64_t subtract[16] = {
    1ll,
    10ll,
    100ll,
    1000ll,
    10000ll,
    100000ll,
    1000000ll,
    10000000ll,
    100000000ll,
    1000000000ll,
    10000000000ll,
    100000000000ll,
    1000000000000ll,
    10000000000000ll,
    100000000000000ll,
    1000000000000000ll };
#define PRINTALL 1

int main()
{
    bool clean = true;
    int64_t last = 0, next = 0;
    double lastd = 0, nextd = 0;
    bool first_run = true;
    for (int32_t exp = maxExponent; exp >= minExponent; exp--)
    {
        int step = 0;
        int substep = 0;
        for (int64_t man = maxMantissa; man >= minMantissa; )
        {
            next = unpack_xfl(make_float(-man, exp));
            nextd = (double)(-man) * std::pow(10, exp);
            if (PRINTALL)
                printf("%ld | -%016ldE%d | %g\n", next, man, exp, nextd);
            if (first_run)
                first_run = false;
            else
            {
                if (last >= next)
                {
                    fprintf(stderr, "exp: %d man: %016ld. last %ld >= next: %ld\n", exp, man, last, next);
                    clean = false;
                }
                
                if (lastd > nextd)
                {
                    fprintf(stderr,
                            "LOGIC ERROR exp: %d man: %016ld. lastd %g >= nextd: %g\n", 
                            exp, man, lastd, nextd);
                    clean = false;
                }
            }
            last = next;
            lastd = nextd;
            

            substep++;
            if (substep == 10)
            {
                substep = 0;
                step++;
                if (step > 15)
                    step = 15;
            }
            man -= subtract[step];
        }
    }
    next = 0;
    nextd = 0;
    if (PRINTALL)
        printf("%ld | -%016ldE%d | %g\n", next, 0L, 0, nextd);
    if (last >= next)
    {
        fprintf(stderr, "exp: %d man: %016ld. last %ld >= next: %ld\n", 0, 0L, last, next);
        clean = false;
    }
    last = next;
    lastd = nextd;

    for (int32_t exp = minExponent; exp <= maxExponent; exp++)
    {
        int step = 0;
        int substep = 0;
        for (int64_t man = minMantissa; man <= maxMantissa; )
        {
            next = unpack_xfl(make_float(man, exp));
            nextd = (double)(man) * std::pow(10, exp);
            if (PRINTALL)
                printf("%ld | +%016ldE%d | %g\n", next, man, exp, nextd);
            if (last >= next)
            {
                fprintf(stderr, "exp: %d man: %016ld. last %ld >= next: %ld\n", exp, man, last, next);
                clean = false;
            }
            if (lastd > nextd)
            {
                fprintf(stderr,
                        "LOGIC ERROR exp: %d man: %016ld. lastd %g >= nextd: %g\n", 
                        exp, man, lastd, nextd);
                clean = false;
            }
            last = next;
            lastd = nextd;

            substep++;
            if (substep == 10)
            {
                substep = 0;
                step++;
                if (step > 15)
                    step = 15;
            }
            man += subtract[step];
        }
    }
    return (clean ? 0 : 1);
}
