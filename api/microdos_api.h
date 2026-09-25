// microdos_api.h
#ifndef MICRODOS_API_H
#define MICRODOS_API_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct TouchState {
  bool isPressed;
  int x;
  int y;
};

typedef struct {
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
  
  // --- Allocator Vectors hooks ---
  void* (*malloc)(unsigned int size);
  void  (*free)(void* ptr);

  // --- Sprite Engine ---
  uint32_t (*createSprite)(const char* filename);
  void  (*drawSprite)(uint32_t spriteHandle, int x, int y);
  void  (*freeSprite)(uint32_t spriteHandle);
  bool  (*initGameMatrix)();
  void  (*flushGameMatrix)();
  void  (*closeGameMatrix)();
} MicroDosAPI;

// Tracking pointer instance to link standard malloc loops cleanly
static MicroDosAPI* _global_api_ptr = 0;

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
#define STRING(str) (__extension__({ \
    static const char __aligned_str[] __attribute__((aligned(4))) = str; \
    __aligned_str; \
}))

#define NEW_STRING(varName, str) \
    static const char varName[] __attribute__((aligned(4))) = str

__attribute__((always_inline)) static inline void padString(char* dest, const char* src, size_t fixedLen) {
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
__attribute__((weak)) int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

__attribute__((weak)) void* memcpy(void* dest, const void* src, unsigned int count) {
  char* d = (char*)dest;
  const char* s = (const char*)src;
  while (count--) {
    *d++ = *s++;
  }
  return dest;
}

__attribute__((weak)) void* memset(void* dest, int value, unsigned int count) {
  char* d = (char*)dest;
  while (count--) {
    *d++ = (char)value;
  }
  return dest;
}

__attribute__((weak)) void* malloc(unsigned int size) {
  if (_global_api_ptr && _global_api_ptr->malloc) {
    return _global_api_ptr->malloc(size);
  }
  return 0; 
}

__attribute__((weak)) void free(void* ptr) {
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

// Main entry point
int _start(int argc, char** argv, MicroDosAPI* api);

// Direct access link to the underlying microkernel structure instance
static inline MicroDosAPI* kernel() {
    return _global_api_ptr;
}

// --- CONSOLE & TERMINAL WRAPPERS ---
__attribute__((always_inline)) static inline void print(const char* text)   { kernel()->print(text); }
__attribute__((always_inline)) static inline void println(const char* text) { kernel()->println(text); }
__attribute__((always_inline)) static inline void clearScreen()             { kernel()->clear(); }
__attribute__((always_inline)) static inline int  readKey()                 { return kernel()->inkey(); }

// --- HARDWARE INTERFACES ---
__attribute__((always_inline)) static inline void delayMs(int ms)           { kernel()->delay(ms); }
__attribute__((always_inline)) static inline void playTone(int freq, int ms){ kernel()->beep(freq, ms); }
__attribute__((always_inline)) static inline void setPinMode(int pin, int m){ kernel()->pinMode(pin, m); }
__attribute__((always_inline)) static inline void writeDigital(int p, int v){ kernel()->digitalWrite(p, v); }
__attribute__((always_inline)) static inline int  readDigital(int pin)     { return kernel()->digitalRead(pin); }

// --- LOW-LEVEL GRAPHICS ENGINE WRAPPERS ---
__attribute__((always_inline)) static inline void setColor(int colorId)     { kernel()->color(colorId); }
__attribute__((always_inline)) static inline void drawPixel(int x, int y, int c) { kernel()->plot(x, y, c); }
__attribute__((always_inline)) static inline void drawLine(int x1, int y1, int x2, int y2, int c) { kernel()->line(x1, y1, x2, y2, c); }
__attribute__((always_inline)) static inline void drawRect(int x, int y, int w, int h, int c)     { kernel()->rect(x, y, w, h, c); }
__attribute__((always_inline)) static inline void drawCircle(int x, int y, int r, int c)         { kernel()->circle(x, y, r, c); }

// --- AUTONOMOUS SPRITE ENGINE WRAPPERS ---
__attribute__((always_inline)) static inline uint32_t loadSprite(const char* file)  { return kernel()->createSprite(file); }
__attribute__((always_inline)) static inline void drawSprite(uint32_t spr, int x, int y) { kernel()->drawSprite(spr, x, y); }
__attribute__((always_inline)) static inline void unloadSprite(uint32_t spr)         { kernel()->freeSprite(spr); }

#ifdef __cplusplus
}
#endif

#endif // MICRODOS_API_H
