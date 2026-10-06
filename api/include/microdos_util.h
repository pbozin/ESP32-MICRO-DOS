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

WEAK void ftoa(float value, char* buffer, int buf_size, int precision) {
    if (buf_size <= 0 || !buffer) return;

    int idx = 0;

    uint32_t u;
    memcpy(&u, &value, 4);
    if ((u & 0x7F800000) == 0x7F800000) {
        const char* special ALIGNED = (u & 0x007FFFFF) ? "NaN" : "Inf";
        if ((u & 0x80000000) && !(u & 0x007FFFFF)) {
            if (idx < buf_size - 1) buffer[idx++] = '-';
        }
        while (*special && idx < buf_size - 1) {
            buffer[idx++] = *special++;
        }
        buffer[idx] = '\0';
        return;
    }

    if (value < 0.0f) {
        if (idx < buf_size - 1) buffer[idx++] = '-';
        value = -value;
    }

    uint32_t int_part = (uint32_t)value;

    float diff = value - (float)int_part;
    uint64_t frac_part = 0;

    if (precision > 0) {
        float scale = 1.0f;
        for (int i = 0; i < precision; i++) scale *= 10.0f;

        frac_part = (uint64_t)(diff * scale + 0.5f);

        if (frac_part >= (uint64_t)scale) {
            frac_part = 0;
            int_part++;
        }
    }

    int int_start = idx;
    if (int_part == 0) {
        if (idx < buf_size - 1) buffer[idx++] = '0';
    } else {
        while (int_part > 0 && idx < buf_size - 1) {
            buffer[idx++] = (char)('0' + (int_part % 10));
            int_part /= 10;
        }
    }
    reverse_str(&buffer[int_start], idx - int_start);

    if (precision > 0 && idx < buf_size - 1) {
        buffer[idx++] = '.';

        int frac_start = idx;
        for (int i = 0; i < precision && idx < buf_size - 1; i++) {
            buffer[idx++] = (char)('0' + (frac_part % 10));
            frac_part /= 10;
        }
        reverse_str(&buffer[frac_start], idx - frac_start);
    }

    buffer[idx < buf_size ? idx : buf_size - 1] = '\0';

    while ((idx % 4) != 0) {
        if (idx < buf_size) {
            buffer[idx++] = '\0';
        } else {
            break; 
        }
    }
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

WEAK int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
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

#ifdef __cplusplus
}
#endif

#endif // MICRODOS_UTIL_H
