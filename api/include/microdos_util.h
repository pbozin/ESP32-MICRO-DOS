// microdos_util.h
#ifndef MICRODOS_UTIL_H
#define MICRODOS_UTIL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
//   RUNTIME METADATA UTILITIES (WEAK LINKAGE)
// ============================================================================
WEAK void padString(char* dest, const char* src, size_t fixedLen) {
    size_t i = 0;
    while (src[i] != '\0' && i < fixedLen) {
        dest[i] = src[i];
        i++;
    }
    while (i < fixedLen) {
        dest[i] = ' ';
        i++;
    }
    dest[fixedLen] = '\0';
}

WEAK char* concat(const char* first, const char* second, char* result) {
    char* ptr ALIGNED = result;
    while (*first)  *ptr++ = *first++;
    while (*second) *ptr++ = *second++;
    *ptr = '\0';
    return result;
}

WEAK void* memcpy(void* dest, const void* src, unsigned int n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    if ((((uintptr_t)d | (uintptr_t)s) & 3) == 0) {
        uint32_t* d32 = (uint32_t*)d;
        const uint32_t* s32 = (const uint32_t*)s;
        while (n >= 4) {
            *d32++ = *s32++;
            n -= 4;
        }
        d = (uint8_t*)d32;
        s = (uint8_t*)s32;
    }

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

WEAK void reverse_str(char* str, int len) {
    int i = 0, j = len - 1;
    while (i < j) {
        char temp ALIGNED = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}

WEAK int atoi(const char* str) {
    int res = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t' || *str == '\n' ||
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        res = (res * 10) + (*str - '0');
        str++;
    }

    return sign * res;
}

WEAK void itoa(uint32_t val, char* buf) {
    int i = 0;
    if (val == 0) { buf[i++] = '0'; buf[i] = '\0'; return; }
    char tmp[12]; int j = 0;
    while (val > 0) {
        tmp[j++] = (val % 10) + '0';
        val /= 10;
    }
    while (j > 0) { buf[i++] = tmp[--j]; }
    buf[i] = '\0';
}

WEAK unsigned int strlen(const char* str) {
    if (str[0] == '\0') return 0;

    const char* s = str;
    while (*s) {
        s++;
    }
    return (unsigned int)(s - str);
}

WEAK void* memset(void* dest, int value, unsigned int count) {
    if (((size_t)dest % 4 == 0) && (count % 4 == 0)) {
        uint32_t* d = (uint32_t*)dest;
        uint32_t val32 = (uint8_t)value;
        val32 |= (val32 << 8);
        val32 |= (val32 << 16);
        unsigned int words = count / 4;
        while (words--) {
            *d++ = val32;
        }
    } else {
        char* d = (char*)dest;
        while (count--) {
            *d++ = (char)value;
        }
    }
    return dest;
}

WEAK int strncmp(const char* s1, const char* s2, size_t n) {
    while (n > 0) {
        if (*s1 != *s2) {
            return *(const unsigned char*)s1 - *(const unsigned char*)s2;
        }
        if (*s1 == '\0') {
            return 0;
        }
        s1++;
        s2++;
        n--;
    }
    return 0;
}

WEAK char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }

    for (; i < n; i++) {
        dest[i] = '\0';
    }

    return dest;
}

WEAK int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

WEAK void* malloc(unsigned int size) {
  if (_global_api_ptr && _global_api_ptr->malloc) {
    return _global_api_ptr->malloc(size);
  }
  return 0;
}

WEAK void free(void* ptr) {
  if (ptr && _global_api_ptr && _global_api_ptr->free) {
    _global_api_ptr->free(ptr);
  }
}

ALWAYS INLINE void ftoa(float in, char* out, int outLen, int precision) {
    if (in < 0.0f) {
        out[0] = '-';
        ftoa(-in, out + 1, outLen - 1, precision);
        return;
    }

    char left[8];
    char right[8];
    char temp[16];

    int lRes = (int)in;
    itoa(lRes, left);

    if (precision <= 0) {
        memcpy(out, left, strlen(left) + 1);
        return;
    }

    int power = 1;
    for (int i = 0; i < precision; ++i) power *= 10;

    float rTemp = (in - (float)lRes) * (float)power;
    int rRes = (int)(rTemp + 0.5f);

    if (rRes >= power) {
        rRes = 0;
        lRes += 1;
        itoa(lRes, left);
    }

    itoa(rRes, right);

    char padded_right[8];
    int right_len = strlen(right);
    int missing_zeros = precision - right_len;

    int p_idx = 0;
    while (missing_zeros > 0 && p_idx < missing_zeros) {
        padded_right[p_idx++] = '0';
    }
    memcpy(padded_right + p_idx, right, right_len + 1);

    concat(left, (char*)STRING("."), temp);
    concat(temp, padded_right, out);
}

#ifdef __cplusplus
}
#endif

#endif // MICRODOS_UTIL_H
