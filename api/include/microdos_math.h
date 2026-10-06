#ifndef MICRODOS_64BIT_H
#define MICRODOS_64BIT_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
   1. 64-BIT BITWISE SHIFTS
   ========================================================================== */

// Arithmetic Shift Left Double Integer (64-bit Left Shift)
WEAK uint64_t __ashldi3(uint64_t a, int b) {
    if (b <= 0) return a;
    if (b >= 64) return 0;
    uint32_t high = static_cast<uint32_t>(a >> 32);
    uint32_t low  = static_cast<uint32_t>(a & 0xFFFFFFFF);
    if (b >= 32) {
        high = low << (b - 32);
        low = 0;
    } else {
        high = (high << b) | (low >> (32 - b));
        low = low << b;
    }
    return (static_cast<uint64_t>(high) << 32) | low;
}

// Logical Shift Right Double Integer (64-bit Unsigned Right Shift)
WEAK uint64_t __lshrdi3(uint64_t a, int b) {
    if (b <= 0) return a;
    if (b >= 64) return 0;
    uint32_t high = static_cast<uint32_t>(a >> 32);
    uint32_t low  = static_cast<uint32_t>(a & 0xFFFFFFFF);
    if (b >= 32) {
        low = high >> (b - 32);
        high = 0;
    } else {
        low = (low >> b) | (high << (32 - b));
        high = high >> b;
    }
    return (static_cast<uint64_t>(high) << 32) | low;
}

// Arithmetic Shift Right Double Integer (64-bit Signed Right Shift)
WEAK int64_t __ashrdi3(int64_t a, int b) {
    if (b <= 0) return a;
    if (b >= 64) return (a < 0) ? -1 : 0;
    uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(a) >> 32);
    uint32_t low  = static_cast<uint32_t>(static_cast<uint64_t>(a) & 0xFFFFFFFF);
    if (b >= 32) {
        low = static_cast<uint32_t>(static_cast<int32_t>(high) >> (b - 32));
        high = (static_cast<int32_t>(high) < 0) ? 0xFFFFFFFF : 0;
    } else {
        low = (low >> b) | (high << (32 - b));
        high = static_cast<uint32_t>(static_cast<int32_t>(high) >> b);
    }
    return static_cast<int64_t>((static_cast<uint64_t>(high) << 32) | low);
}


/* ==========================================================================
   2. 64-BIT ARITHMETIC (DIVISION & MODULO)
   ========================================================================== */

// Unsigned 64-bit Division
WEAK uint64_t __udivdi3(uint64_t num, uint64_t den) {
    if (den == 0) return 0;
    uint64_t quot = 0, rem = 0;
    for (int i = 63; i >= 0; i--) {
        rem = (rem << 1) | ((num >> i) & 1);
        if (rem >= den) {
            rem -= den;
            quot |= (1ULL << i);
        }
    }
    return quot;
}

// Unsigned 64-bit Modulo
WEAK uint64_t __umoddi3(uint64_t num, uint64_t den) {
    if (den == 0) return 0;
    uint64_t rem = 0;
    for (int i = 63; i >= 0; i--) {
        rem = (rem << 1) | ((num >> i) & 1);
        if (rem >= den) rem -= den;
    }
    return rem;
}

// Signed 64-bit Division
WEAK int64_t __divdi3(int64_t a, int64_t b) {
    uint64_t ua = (a < 0) ? -static_cast<uint64_t>(a) : static_cast<uint64_t>(a);
    uint64_t ub = (b < 0) ? -static_cast<uint64_t>(b) : static_cast<uint64_t>(b);
    uint64_t uquot = __udivdi3(ua, ub);
    if ((a < 0) ^ (b < 0)) return -static_cast<int64_t>(uquot);
    return static_cast<int64_t>(uquot);
}

// Signed 64-bit Modulo
WEAK int64_t __moddi3(int64_t a, int64_t b) {
    uint64_t ua = (a < 0) ? -static_cast<uint64_t>(a) : static_cast<uint64_t>(a);
    uint64_t ub = (b < 0) ? -static_cast<uint64_t>(b) : static_cast<uint64_t>(b);
    uint64_t urem = __umoddi3(ua, ub);
    if (a < 0) return -static_cast<int64_t>(urem);
    return static_cast<int64_t>(urem);
}


/* ==========================================================================
   3. FLOAT & 64-BIT INT CONVERSIONS
   ========================================================================== */

// Signed 64-bit Integer to 32-bit Float
WEAK float __floatdisf(int64_t a) {
    if (a == 0) return 0.0f;
    uint32_t sign = 0;
    uint64_t ua = a;
    if (a < 0) {
        sign = 0x80000000;
        ua = -static_cast<uint64_t>(a);
    }
    int msb = 63;
    while ((ua & (1ULL << msb)) == 0) msb--;
    int32_t exp = 127 + msb;
    uint32_t man = 0;
    if (msb <= 23) {
        man = static_cast<uint32_t>(ua << (23 - msb));
    } else {
        man = static_cast<uint32_t>(ua >> (msb - 23));
        if ((ua >> (msb - 24)) & 1) {
            man++;
            if (man & 0x01000000) { man >>= 1; exp++; }
        }
    }
    uint32_t res_u = sign | ((exp & 0xFF) << 23) | (man & 0x007FFFFF);
    float result;
    memcpy(&result, &res_u, 4);
    return result;
}

// Unsigned 64-bit Integer to 32-bit Float
WEAK float __floatundisf(uint64_t a) {
    if (a == 0) return 0.0f;
    int msb = 63;
    while ((a & (1ULL << msb)) == 0) msb--;
    int32_t exp = 127 + msb;
    uint32_t man = 0;
    if (msb <= 23) {
        man = static_cast<uint32_t>(a << (23 - msb));
    } else {
        man = static_cast<uint32_t>(a >> (msb - 23));
        if ((a >> (msb - 24)) & 1) {
            man++;
            if (man & 0x01000000) { man >>= 1; exp++; }
        }
    }
    uint32_t res_u = ((exp & 0xFF) << 23) | (man & 0x007FFFFF);
    float result;
    memcpy(&result, &res_u, 4);
    return result;
}

// 32-bit Float to Signed 64-bit Integer
WEAK int64_t __fixsfdi(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    int32_t exp = ((u >> 23) & 0xFF) - 127;
    if (exp < 0) return 0;
    uint64_t man = (u & 0x007FFFFF) | 0x00800000;
    uint64_t res;
    if (exp <= 23) res = man >> (23 - exp);
    else           res = man << (exp - 23);
    return ((u & 0x80000000) ? -static_cast<int64_t>(res) : static_cast<int64_t>(res));
}

// 32-bit Float to Unsigned 64-bit Integer
WEAK uint64_t __fixunssfdi(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    if (u & 0x80000000) return 0;
    int32_t exp = ((u >> 23) & 0xFF) - 127;
    if (exp < 0) return 0;
    uint64_t man = (u & 0x007FFFFF) | 0x00800000;
    if (exp <= 23) return man >> (23 - exp);
    return man << (exp - 23);
}


/* ==========================================================================
   4. SINGLE PRECISION SOFT-FLOAT DIVISION
   ========================================================================== */

// Float Division
WEAK float __divsf3(float a, float b) {
    uint32_t ua, ub;
    memcpy(&ua, &a, 4);
    memcpy(&ub, &b, 4);
    uint32_t sign_a = ua & 0x80000000;
    uint32_t sign_b = ub & 0x80000000;
    int32_t exp_a = (ua >> 23) & 0xFF;
    int32_t exp_b = (ub >> 23) & 0xFF;

    if (ub == 0 || (exp_b == 0 && (ub & 0x007FFFFF) == 0)) {
        uint32_t inf = (sign_a ^ sign_b) | 0x7F800000;
        float inf_f; memcpy(&inf_f, &inf, 4); return inf_f;
    }

    uint64_t man_a = (ua & 0x007FFFFF) | 0x00800000;
    uint64_t man_b = (ub & 0x007FFFFF) | 0x00800000;
    uint32_t res_sign = sign_a ^ sign_b;
    int32_t res_exp = exp_a - exp_b + 127;

    uint64_t res_man = __udivdi3(__ashldi3(man_a, 23), man_b);

    if (res_man & 0x01000000) {
        res_man >>= 1;
        res_exp++;
    } else if (!(res_man & 0x00800000) && res_man > 0) {
        res_man <<= 1;
        res_exp--;
    }
    if (res_exp <= 0) return 0.0f;
    if (res_exp >= 255) {
        uint32_t inf = res_sign | 0x7F800000;
        float inf_f; memcpy(&inf_f, &inf, 4); return inf_f;
    }

    uint32_t res_u = res_sign | ((res_exp & 0xFF) << 23) | (res_man & 0x007FFFFF);
    float result;
    memcpy(&result, &res_u, 4);
    return result;
}

WEAK double __divdf3(double a, double b) {
    uint64_t ua, ub;
    memcpy(&ua, &a, 8);
    memcpy(&ub, &b, 8);

    uint64_t sign_a = ua & 0x8000000000000000ULL;
    uint64_t sign_b = ub & 0x8000000000000000ULL;
    int64_t exp_a = (ua >> 52) & 0x7FF;
    int64_t exp_b = (ub >> 52) & 0x7FF;

    uint64_t res_sign = sign_a ^ sign_b;

    if (ub == 0 || (exp_b == 0 && (ub & 0x000FFFFFFFFFFFFFULL) == 0)) {
        uint64_t inf = res_sign | 0x7FF0000000000000ULL;
        double inf_d; memcpy(&inf_d, &inf, 8); return inf_d;
    }

    uint64_t man_a = (ua & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;
    uint64_t man_b = (ub & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;

    int64_t res_exp = exp_a - exp_b + 1023;

    uint64_t quot = 0, rem = man_a;
    for (int i = 53; i >= 0; i--) {
        if (rem >= man_b) {
            rem -= man_b;
            quot |= (1ULL << i);
        }
        rem <<= 1;
    }

    if (quot & 0x0020000000000000ULL) {
        quot >>= 1;
        res_exp++;
    } else if (!(quot & 0x0010000000000000ULL) && quot > 0) {
        quot <<= 1;
        res_exp--;
    }
    if (res_exp <= 0) return 0.0;
    if (res_exp >= 2047) {
        uint64_t inf = res_sign | 0x7FF0000000000000ULL;
        double inf_d; memcpy(&inf_d, &inf, 8); return inf_d;
    }
    uint64_t res_u = res_sign | ((res_exp & 0x7FF) << 52) | (quot & 0x000FFFFFFFFFFFFFULL);
    double result;
    memcpy(&result, &res_u, 8);
    return result;
}

// Signed 64-bit Integer to 64-bit Double
WEAK double __floatdidf(int64_t a) {
    if (a == 0) return 0.0;
    uint64_t sign = 0;
    uint64_t ua = a;
    if (a < 0) {
        sign = 0x8000000000000000ULL;
        ua = -static_cast<uint64_t>(a);
    }
    int msb = 63;
    while ((ua & (1ULL << msb)) == 0) msb--;
    int64_t exp = 1023 + msb;
    uint64_t man = 0;
    if (msb <= 52) {
        man = ua << (52 - msb);
    } else {
        man = ua >> (msb - 52);
        if ((ua >> (msb - 53)) & 1) {
            man++;
            if (man & 0x0020000000000000ULL) { man >>= 1; exp++; }
        }
    }
    uint64_t res_u = sign | ((exp & 0x7FF) << 52) | (man & 0x000FFFFFFFFFFFFFULL);
    double result; memcpy(&result, &res_u, 8); return result;
}

// Unsigned 64-bit Integer to 64-bit Double
WEAK double __floatundidf(uint64_t a) {
    if (a == 0) return 0.0;
    int msb = 63;
    while ((a & (1ULL << msb)) == 0) msb--;
    int64_t exp = 1023 + msb;
    uint64_t man = 0;
    if (msb <= 52) {
        man = a << (52 - msb);
    } else {
        man = a >> (msb - 52);
        if ((a >> (msb - 53)) & 1) {
            man++;
            if (man & 0x0020000000000000ULL) { man >>= 1; exp++; }
        }
    }
    uint64_t res_u = ((exp & 0x7FF) << 52) | (man & 0x000FFFFFFFFFFFFFULL);
    double result; memcpy(&result, &res_u, 8); return result;
}

// 64-bit Double to Signed 64-bit Integer
WEAK int64_t __fixdfdi(double d) {
    uint64_t u; memcpy(&u, &d, 8);
    int64_t exp = ((u >> 52) & 0x7FF) - 1023;
    if (exp < 0) return 0;
    uint64_t man = (u & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;
    uint64_t res;
    if (exp <= 52) res = man >> (52 - exp);
    else res = man << (exp - 52);
    return ((u & 0x8000000000000000ULL) ? -static_cast<int64_t>(res) : static_cast<int64_t>(res));
}

// 64-bit Double to Unsigned 64-bit Integer
WEAK uint64_t __fixunsdfdi(double d) {
    uint64_t u; memcpy(&u, &d, 8);
    if (u & 0x8000000000000000ULL) return 0;
    int64_t exp = ((u >> 52) & 0x7FF) - 1023;
    if (exp < 0) return 0;
    uint64_t man = (u & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;
    if (exp <= 52) return man >> (52 - exp);
    return man << (exp - 52);
}

// Extend Single Precision Float to Double Precision (__extendsfdf2)
WEAK double __extendsfdf2(float a) {
    uint32_t ua; memcpy(&ua, &a, 4);
    uint64_t sign = static_cast<uint64_t>(ua & 0x80000000) << 32;
    int32_t exp = (ua >> 23) & 0xFF;
    uint32_t man = ua & 0x007FFFFF;

    if (exp == 0 && man == 0) {
        double zero; uint64_t zbits = sign; memcpy(&zero, &zbits, 8); return zero;
    }
    int64_t new_exp = (exp == 255) ? 2047 : (exp - 127 + 1023);
    uint64_t new_man = static_cast<uint64_t>(man) << (52 - 23);
    uint64_t res_u = sign | (new_exp << 52) | new_man;
    double result; memcpy(&result, &res_u, 8); return result;
}

// Truncate Double Precision to Single Precision (__truncdfsf2)
WEAK float __truncdfsf2(double a) {
    uint64_t ua; memcpy(&ua, &a, 8);
    uint32_t sign = static_cast<uint32_t>(ua >> 32) & 0x80000000;
    int64_t exp = (ua >> 52) & 0x7FF;
    uint64_t man = ua & 0x000FFFFFFFFFFFFFULL;
    if (exp == 0 && man == 0) {
        float zero; memcpy(&zero, &sign, 4); return zero;
    }
    int32_t new_exp = exp - 1023 + 127;
    if (new_exp <= 0) return 0.0f;
    if (new_exp >= 255 || exp == 2047) {
        uint32_t inf = sign | 0x7F800000;
        float inf_f; memcpy(&inf_f, &inf, 4); return inf_f;
    }
    uint32_t new_man = static_cast<uint32_t>(man >> (52 - 23));
    uint32_t res_u = sign | ((new_exp & 0xFF) << 23) | (new_man & 0x007FFFFF);
    float result; memcpy(&result, &res_u, 4); return result;
}

/* ==========================================================================
   5. 32-BIT ARITHMETIC (DIVISION & MODULO)
   ========================================================================== */

// Unsigned 32-bit Division
WEAK uint32_t __udivsi3(uint32_t num, uint32_t den) {
    if (den == 0) return 0;
    uint32_t quot = 0, rem = 0;
    for (int i = 31; i >= 0; i--) {
        rem = (rem << 1) | ((num >> i) & 1);
        if (rem >= den) {
            rem -= den;
            quot |= (1U << i);
        }
    }
    return quot;
}

// Unsigned 32-bit Modulo
WEAK uint32_t __umodsi3(uint32_t num, uint32_t den) {
    if (den == 0) return 0;
    uint32_t rem = 0;
    for (int i = 31; i >= 0; i--) {
        rem = (rem << 1) | ((num >> i) & 1);
        if (rem >= den) rem -= den;
    }
    return rem;
}

// Signed 32-bit Division
WEAK int32_t __divsi3(int32_t a, int32_t b) {
    uint32_t ua = (a < 0) ? -static_cast<uint32_t>(a) : static_cast<uint32_t>(a);
    uint32_t ub = (b < 0) ? -static_cast<uint32_t>(b) : static_cast<uint32_t>(b);
    uint32_t uquot = __udivsi3(ua, ub);
    if ((a < 0) ^ (b < 0)) return -static_cast<int32_t>(uquot);
    return static_cast<int32_t>(uquot);
}

// Signed 32-bit Modulo
WEAK int32_t __modsi3(int32_t a, int32_t b) {
    uint32_t ua = (a < 0) ? -static_cast<uint32_t>(a) : static_cast<uint32_t>(a);
    uint32_t ub = (b < 0) ? -static_cast<uint32_t>(b) : static_cast<uint32_t>(b);
    uint32_t urem = __umodsi3(ua, ub);
    if (a < 0) return -static_cast<int32_t>(urem);
    return static_cast<int32_t>(urem);
}

/* ==========================================================================
   6. FLOAT & 32-BIT INT CONVERSIONS
   ========================================================================== */

// Signed 32-bit Integer to 32-bit Float (__floatsisf)
WEAK float __floatsisf(int32_t a) {
    if (a == 0) return 0.0f;
    uint32_t sign = 0;
    uint32_t ua = a;
    if (a < 0) {
        sign = 0x80000000;
        ua = -static_cast<uint32_t>(a);
    }
    int msb = 31;
    while ((ua & (1U << msb)) == 0) msb--;
    int32_t exp = 127 + msb;
    uint32_t man = 0;
    if (msb <= 23) {
        man = ua << (23 - msb);
    } else {
        man = ua >> (msb - 23);
        if ((ua >> (msb - 24)) & 1) {
            man++;
            if (man & 0x01000000) { man >>= 1; exp++; }
        }
    }
    uint32_t res_u = sign | ((exp & 0xFF) << 23) | (man & 0x007FFFFF);
    float result; memcpy(&result, &res_u, 4); return result;
}

// Unsigned 32-bit Integer to 32-bit Float (__floatunsisf)
WEAK float __floatunsisf(uint32_t a) {
    if (a == 0) return 0.0f;
    int msb = 31;
    while ((a & (1U << msb)) == 0) msb--;
    int32_t exp = 127 + msb;
    uint32_t man = 0;
    if (msb <= 23) {
        man = a << (23 - msb);
    } else {
        man = a >> (msb - 23);
        if ((a >> (msb - 24)) & 1) {
            man++;
            if (man & 0x01000000) { man >>= 1; exp++; }
        }
    }
    uint32_t res_u = ((exp & 0xFF) << 23) | (man & 0x007FFFFF);
    float result; memcpy(&result, &res_u, 4); return result;
}

// 32-bit Float to Signed 32-bit Integer (__fixsfsi)
WEAK int32_t __fixsfsi(float f) {
    uint32_t u; memcpy(&u, &f, 4);
    int32_t exp = ((u >> 23) & 0xFF) - 127;
    if (exp < 0) return 0;
    if (exp > 30) return (u & 0x80000000) ? INT32_MIN : INT32_MAX;
    uint32_t man = (u & 0x007FFFFF) | 0x00800000;
    uint32_t res;
    if (exp <= 23) res = man >> (23 - exp);
    else           res = man << (exp - 23);
    return ((u & 0x80000000) ? -static_cast<int32_t>(res) : static_cast<int32_t>(res));
}

// 32-bit Float to Unsigned 32-bit Integer (__fixunssfsi)
WEAK uint32_t __fixunssfsi(float f) {
    uint32_t u; memcpy(&u, &f, 4);
    if (u & 0x80000000) return 0;
    int32_t exp = ((u >> 23) & 0xFF) - 127;
    if (exp < 0) return 0;
    if (exp > 31) return UINT32_MAX;
    uint32_t man = (u & 0x007FFFFF) | 0x00800000;
    if (exp <= 23) return man >> (23 - exp);
    return man << (exp - 23);
}

WEAK float __mulsf3(float a, float b) {
    uint32_t ua, ub;
    memcpy(&ua, &a, 4);
    memcpy(&ub, &b, 4);

    uint32_t sign_a = ua & 0x80000000;
    uint32_t sign_b = ub & 0x80000000;
    int32_t exp_a = (ua >> 23) & 0xFF;
    int32_t exp_b = (ub >> 23) & 0xFF;

    if (exp_a == 0 || exp_b == 0) {
        float zero = 0.0f;
        uint32_t zbits = sign_a ^ sign_b;
        memcpy(&zero, &zbits, 4);
        return zero;
    }

    uint64_t man_a = (ua & 0x007FFFFF) | 0x00800000;
    uint64_t man_b = (ub & 0x007FFFFF) | 0x00800000;

    uint32_t res_sign = sign_a ^ sign_b;
    int32_t res_exp = exp_a + exp_b - 127;

    uint64_t res_man = man_a * man_b;

    if (res_man & 0x020000000000ULL) {
        res_man >>= 24;
        res_exp += 1;
    } else {
        res_man >>= 23;
    }

    if (res_exp <= 0) return 0.0f;
    if (res_exp >= 255) {
        uint32_t inf = res_sign | 0x7F800000;
        float inf_f; memcpy(&inf_f, &inf, 4); return inf_f;
    }

    uint32_t res_u = res_sign | ((res_exp & 0xFF) << 23) | (static_cast<uint32_t>(res_man) & 0x007FFFFF);
    float result;
    memcpy(&result, &res_u, 4);
    return result;
}

#ifdef __cplusplus
}
#endif
#endif // MICRODOS_64BIT_H

