#ifndef MICRODOS_MATH_H
#define MICRODOS_MATH_H

//#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
   64-BIT BITWISE SHIFTS
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
   64-BIT ARITHMETIC (DIVISION & MODULO)
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

#ifdef __cplusplus
}
#endif
#endif // MICRODOS_MATH_H

