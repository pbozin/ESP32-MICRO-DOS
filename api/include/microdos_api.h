// microdos_api.h
#ifndef MICRODOS_API_H
#define MICRODOS_API_H

#include <stdint.h>
#include <stddef.h>

#define ALIGNED __attribute__((aligned(4)))
#define WEAK __attribute__((weak))
#define INLINE static inline
#define ALWAYS __attribute__((always_inline))

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


#define INPUT 0x01
#define OUTPUT 0x03
#define INPUT_PULLUP 0x05
#define INPUT_PULLDOWN 0x09

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
  void (*setFKeys)(const char* l1,
		   const char* l2,
		   const char* l3,
		   const char* l4,
		   const char* l5);
  void (*clearFKeys)();

  // --- Allocator Vectors Hooks ---
  void* (*malloc)(unsigned int size);
  void  (*free)(void* ptr);

  // --- Sprite Engine ---
  uint32_t (*createSprite)(const char* filename, int spriteSize);
  void  (*drawSprite)(uint32_t spriteHandle, int x, int y);
  void  (*clearSprite)(uint32_t spriteHandle);
  void  (*freeSprite)(uint32_t spriteHandle);
  bool  (*initGameMatrix)();
  void  (*flushGameMatrix)();
  void  (*closeGameMatrix)();

  // --- Serial Debugger ---
  void  (*sysDebugDump)(const char* label,
		        const void* memoryAddress,
			unsigned int byteCount,
			uint32_t virtualAddr);

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

// --- CONSOLE & TERMINAL WRAPPERS ---
ALWAYS INLINE void setFKeys(const char* l1,
		            const char* l2,
			    const char* l3,
			    const char* l4,
			    const char* l5)      { kernel()->setFKeys(l1, l2, l3, l4, l5); }
ALWAYS INLINE void termPrintAt(int x,
		               int y,
			       const char* text) { kernel()->printAt(x, y, text); }
ALWAYS INLINE void inputStr(const char* prompt,
		            char* destBuffer,
			    int maxLen)          { kernel()->inputStr(prompt, destBuffer, maxLen); }
ALWAYS INLINE void termPrint(const char* text)   { kernel()->print(text); }
ALWAYS INLINE void termPrintln(const char* text) { kernel()->println(text); }
ALWAYS INLINE void termClear()                   { kernel()->clear(); }
ALWAYS INLINE void getTouch(TouchState* state)   { kernel()->getTouch(state); }
ALWAYS INLINE void clearFKeys()                  { kernel()->clearFKeys(); }
ALWAYS INLINE int  getKey()                      { return kernel()->inkey(); }
ALWAYS INLINE int  getTermWidth()                { return kernel()->termWidth; }
ALWAYS INLINE int  getTermHeight()               { return kernel()->termHeight; }
ALWAYS INLINE int  getCharWidth()                { return kernel()->charWidth; }
ALWAYS INLINE int  getCharHeight()               { return kernel()->charHeight; }

// --- HARDWARE INTERFACES ---
ALWAYS INLINE int  wifiUp(const char* ssid,
		          const char* pass)      { return kernel()->wifiUp(ssid, pass); }
ALWAYS INLINE void wifiDown()                    { kernel()->wifiDown(); }
ALWAYS INLINE int  getRamSize()                  { return kernel()->getRamSize(); }
ALWAYS INLINE void poke(int address, int value)  { kernel()->poke(address, value); }
ALWAYS INLINE int  peek(int address)             { return kernel()->peek(address); }
ALWAYS INLINE void playSound(int freq, int ms)   { kernel()->beep(freq, ms); }
ALWAYS INLINE void delayMs(int ms)               { kernel()->delay(ms); }
ALWAYS INLINE void pinMode(int pin, int m)       { kernel()->pinMode(pin, m); }
ALWAYS INLINE void digitalWrite(int p, int v)    { kernel()->digitalWrite(p, v); }
ALWAYS INLINE int  digitalRead(int pin)          { return kernel()->digitalRead(pin); }

// --- LOW-LEVEL GRAPHICS ENGINE WRAPPERS ---
ALWAYS INLINE void setColor(int colorId)                           { kernel()->color(colorId); }
ALWAYS INLINE void drawPixel(int x, int y, int c)                  { kernel()->plot(x, y, c); }
ALWAYS INLINE void drawLine(int x1, int y1, int x2, int y2, int c) { kernel()->line(x1, y1, x2, y2, c); }
ALWAYS INLINE void drawRect(int x, int y, int w, int h, int c)     { kernel()->rect(x, y, w, h, c); }
ALWAYS INLINE void drawCircle(int x, int y, int r, int c)          { kernel()->circle(x, y, r, c); }
ALWAYS INLINE void drawJpeg(const char* filename, int x, int y)    { kernel()->drawJpeg(filename, x, y); }

// --- AUTONOMOUS SPRITE ENGINE WRAPPERS ---
ALWAYS INLINE void drawSprite(uint32_t spr,
		              int x,
			      int y)            { kernel()->drawSprite(spr, x, y); }
ALWAYS INLINE uint32_t createSprite(const char* file,
		                    int size)   { return kernel()->createSprite(file, size); }
ALWAYS INLINE void clearSprite(uint32_t spr)    { kernel()->clearSprite(spr); }
ALWAYS INLINE void freeSprite(uint32_t spr)     { kernel()->freeSprite(spr); }
ALWAYS INLINE bool initGameMatrix()             { return kernel()->initGameMatrix(); }
ALWAYS INLINE void flushGameMatrix()            { kernel()->flushGameMatrix(); }
ALWAYS INLINE void closeGameMatrix()            { kernel()->closeGameMatrix(); }

// --- SERIAL WRAPPERS ---
ALWAYS INLINE int  serialOpen(uint32_t baud,
		              int txPin,
			      int rxPin)              { return kernel()->serialOpen(baud, txPin, rxPin); }
ALWAYS INLINE void serialWrite(const uint8_t* buffer,
		               unsigned int length)   { kernel()->serialWrite(buffer, length); }
ALWAYS INLINE int  serialRead(uint8_t* buffer,
		              unsigned int maxLength) { return kernel()->serialRead(buffer, maxLength); }
ALWAYS INLINE void serialClose()                      { kernel()->serialClose(); }

// --- MISC WRAPPERS ---
ALWAYS INLINE int getRandom(int min, int max)   { return kernel()->random(min, max); }

ALWAYS INLINE int ollamaStream(const char* prompt,
		               const char* serverIp,
			       const char* modelName,
			       const char* sysPrompt,
			       int streamToConsole) {
	                           return kernel()->ollamaStream(prompt,
				                                 serverIp,
								 modelName,
								 sysPrompt,
								 streamToConsole);
                               }
ALWAYS INLINE void kernelDebug(const char* lbl,
		               const void* ptr,
			       unsigned int len,
			       uint32_t vAddr) {
                                   if (kernel()->sysDebugDump) {
                                        kernel()->sysDebugDump(lbl, ptr, len, vAddr);
                                   }
                               }
// Main entry point
int _start(int argc, char** argv, MicroDosAPI* api);

#ifdef __cplusplus
}
#endif

#endif // MICRODOS_API_H
