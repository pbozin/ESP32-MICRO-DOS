// microdos_api.h
#ifndef MICRODOS_API_H
#define MICRODOS_API_H

#include <stdint.h>
#include <stddef.h>

#define ALIGNED __attribute__((aligned(4)))
#define WEAK __attribute__((weak))
#define INLINE static inline
#define ALWAYS __attribute__((always_inline))

#ifdef __cplusplus
extern "C" {
#endif

struct TouchState {
  bool isPressed;
  int x;
  int y;
};

struct MicroDosAPI {
  // --- Console Printing Vectors ---
  void (*print)(const char* text);
  void (*println)(const char* text);
  void (*clear)();

  // --- Embedded Hardware Signals ---
  void (*beep)(int freq, int ms);
  void (*delay)(int ms);
  int  (*inkey)();

  // --- Low-Level Vector Graphics ---
  void (*color)(int colorId);
  void (*plot)(int x, int y, int colorId);
  void (*line)(int x1, int y1, int x2, int y2, int colorId);
  void (*rect)(int x, int y, int w, int h, int colorId);
  void (*circle)(int x, int y, int r, int colorId);

  // --- Direct Memory Sandboxing ---
  int  (*peek)(int address);
  void (*poke)(int address, int value);
  int  (*getRamSize)();

  // --- Protected GPIO Fences ---
  void (*pinMode)(int pin, int mode);
  void (*digitalWrite)(int pin, int val);
  int  (*digitalRead)(int pin);

  // --- System Input & Network Control ---
  void (*inputStr)(const char* prompt, char* destBuffer, int maxLen);
  int  (*wifiUp)(const char* ssid, const char* pass);
  void (*wifiDown)();

  // --- Local Autonomous AI Streaming ---
  int  (*ollamaStream)(const char* prompt,
		       const char* serverIp,
		       const char* modelName,
		       const char* sysPrompt,
		       int streamToConsole);

  // --- Touch Panel Input Engine ---
  void (*getTouch)(TouchState* state);
  int termWidth;
  int termHeight;
  int charWidth;
  int charHeight;

  // --- Extended Visual Add-ons ---
  void (*drawJpeg)(const char* filename, int x, int y);
  void (*setFKeys)(const char* l1, const char* l2, const char* l3, const char* l4, const char* l5);
  void (*clearFKeys)();

  // --- Allocator Vectors Hooks ---
  void* (*malloc)(unsigned int size);
  void  (*free)(void* ptr);

  // --- Sprite Engine ---
  uint32_t (*createSprite)(const char* filename, int spriteSize);
  void  (*drawSprite)(uint32_t spriteHandle, int x, int y);
  void  (*freeSprite)(uint32_t spriteHandle);
  bool  (*initGameMatrix)();
  void  (*flushGameMatrix)();
  void  (*closeGameMatrix)();

  // --- Serial Debugger ---
  void  (*sysDebugDump)(const char* label, const void* memoryAddress, unsigned int byteCount, uint32_t virtualAddr);

  // --- Misc ---
  void (*printAt)(int x, int y, const char* text);
  int  (*random)(int min, int max);

  // --- Serial Port Handling ---
  int  (*serialOpen)(uint32_t baud, int txPin, int rxPin);
  void (*serialWrite)(const uint8_t* buffer, unsigned int length);
  int  (*serialRead)(uint8_t* buffer, unsigned int maxLength);
  void (*serialClose)();
};

// Tracking pointer instance to link standard malloc lodops cleanly
WEAK MicroDosAPI* _global_api_ptr = 0;

// ============================================================================
//   4BIT COLOR PALETTE
// ============================================================================
#define BLACK     0
#define WHITE     1
#define LIGHTGREY 2
#define RED       3
#define ORANGE    4
#define YELLOW    5
#define GREEN     6
#define CYAN      7
#define BLUE      8
#define MAGENTA   9
#define MAROON    10
#define DARKGREEN 11
#define DARKCYAN  12
#define NAVY      13
#define PINK      14

// ============================================================================
//   STRING ALIGNMENT AND CREATION UTILITIES
// ============================================================================

#undef STRING
#define STRING(str) (__extension__({ \
    struct ALIGNED AlignedStrWrapper { \
        char data[sizeof(str)]; \
    }; \
    static const struct AlignedStrWrapper __wrapped_str = { str }; \
    __wrapped_str.data; \
}))

#define NEW_STRING(varName, str) \
    static const char varName[] ALIGNED = str

ALWAYS INLINE void padString(char* dest, const char* src, size_t fixedLen) {
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

// ============================================================================
//   RUNTIME METADATA UTILITIES (WEAK LINKAGE)
// ============================================================================

ALWAYS INLINE char* concat(const char* first, const char* second, char* result) {
    char* ptr = result;
    while (*first)  *ptr++ = *first++;
    while (*second) *ptr++ = *second++;
    *ptr = '\0';
    return result;
}

WEAK int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

WEAK void* memcpy(void* dest, const void* src, unsigned int count) {
    if (((size_t)dest % 4 == 0) && ((size_t)src % 4 == 0) && (count % 4 == 0)) {
        uint32_t* d = (uint32_t*)dest;
        const uint32_t* s = (const uint32_t*)src;
        unsigned int words = count / 4;
        while (words--) {
            *d++ = *s++;
        }
    } else {
        char* d = (char*)dest;
        const char* s = (const char*)src;
        while (count--) {
            *d++ = *s++;
        }
    }
    return dest;
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

// ============================================================================
//   GUEST-SIDE REUSABLE TOUCH LAYOUT MACROS
// ============================================================================
#define IS_TOUCH_IN_CANVAS(state, api) \
    ((state).isPressed && (state).x >= 0 && (state).x < (api)->termWidth && \
     (state).y >= 0 && (state).y < (api)->termHeight)

#define TOUCH_TO_GRID_X(state, api) ((state).x / (api)->charWidth)
#define TOUCH_TO_GRID_Y(state, api) ((state).y / (api)->charHeight)

#define TOUCH_IN_BOUNDS(state, bx, by, bw, bh) \
    ((state).isPressed && (state).x >= (bx) && (state).x < ((bx) + (bw)) && \
     (state).y >= (by) && (state).y < ((by) + (bh)))

INLINE MicroDosAPI* kernel() {
    return _global_api_ptr;
}

ALWAYS INLINE void kernelDebug(const char* lbl, const void* ptr, unsigned int len, uint32_t vAddr) {
    if (kernel()->sysDebugDump) {
        kernel()->sysDebugDump(lbl, ptr, len, vAddr);
    }
}

// --- CONSOLE & TERMINAL WRAPPERS ---
ALWAYS INLINE void print(const char* text)    { kernel()->print(text); }
ALWAYS INLINE void println(const char* text)  { kernel()->println(text); }
ALWAYS INLINE void clearScreen()              { kernel()->clear(); }
ALWAYS INLINE int  readKey()                  { return kernel()->inkey(); }

// --- HARDWARE INTERFACES ---
ALWAYS INLINE void delayMs(int ms)            { kernel()->delay(ms); }
ALWAYS INLINE void playTone(int freq, int ms) { kernel()->beep(freq, ms); }
ALWAYS INLINE void setPinMode(int pin, int m) { kernel()->pinMode(pin, m); }
ALWAYS INLINE void writeDigital(int p, int v) { kernel()->digitalWrite(p, v); }
ALWAYS INLINE int  readDigital(int pin)       { return kernel()->digitalRead(pin); }

// --- LOW-LEVEL GRAPHICS ENGINE WRAPPERS ---
ALWAYS INLINE void setColor(int colorId)                           { kernel()->color(colorId); }
ALWAYS INLINE void drawPixel(int x, int y, int c)                  { kernel()->plot(x, y, c); }
ALWAYS INLINE void drawLine(int x1, int y1, int x2, int y2, int c) { kernel()->line(x1, y1, x2, y2, c); }
ALWAYS INLINE void drawRect(int x, int y, int w, int h, int c)     { kernel()->rect(x, y, w, h, c); }
ALWAYS INLINE void drawCircle(int x, int y, int r, int c)          { kernel()->circle(x, y, r, c); }

// --- AUTONOMOUS SPRITE ENGINE WRAPPERS ---
ALWAYS INLINE uint32_t loadSprite(const char* file, int size)      { return kernel()->createSprite(file, size); }
ALWAYS INLINE void drawSprite(uint32_t spr, int x, int y)          { kernel()->drawSprite(spr, x, y); }
ALWAYS INLINE void unloadSprite(uint32_t spr)                      { kernel()->freeSprite(spr); }

// --- MISC WRAPPERS ---
ALWAYS INLINE int random(int min, int max)                         { return kernel()->random(min, max); }
ALWAYS INLINE void delay(int ms)                                   { kernel()->delay(ms); }
ALWAYS INLINE void printAt(int x, int y, const char* text)         { kernel()->printAt(x, y, text); }

// --- SERIAL WRAPPERS ---
ALWAYS INLINE int  serialOpen(uint32_t baud, int txPin, int rxPin) { return kernel()->serialOpen(baud, txPin, rxPin); }
ALWAYS INLINE void serialWrite(const uint8_t* buffer, unsigned int length) { kernel()->serialWrite(buffer, length); }
ALWAYS INLINE int  serialRead(uint8_t* buffer, unsigned int maxLength) { return kernel()->serialRead(buffer, maxLength); }
ALWAYS INLINE void serialClose()                                   { kernel()->serialClose(); }

// Main entry point
int _start(int argc, char** argv, MicroDosAPI* api);

#ifdef __cplusplus
}
#endif

#endif // MICRODOS_API_H
