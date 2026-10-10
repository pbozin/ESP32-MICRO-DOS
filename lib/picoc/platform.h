/* all platform-specific includes and defines go in this file */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <setjmp.h>
#include <math.h>
#include <stdbool.h>

#define ESP32_HOST

typedef struct TouchState_t { bool isPressed; int x; int y; } TouchState;

typedef struct MicroDosAPI_t {
  void (*print)(const char* text);
  void (*println)(const char* text);
  void (*clear)();
  void (*beep)(int freq, int ms);
  void (*delay)(int ms);
  int  (*inkey)();
  void (*color)(int colorId);
  void (*plot)(int x, int y, int colorId);
  void (*drawLine)(int x1, int y1, int x2, int y2, int colorId);
  void (*drawRect)(int x, int y, int w, int h, int colorId);
  void (*fillRect)(int x, int y, int w, int h, int colorId);
  void (*drawCircle)(int x, int y, int r, int colorId);
  void (*fillCircle)(int x, int y, int r, int colorId);
  int  (*peek)(int address);
  void (*poke)(int address, int value);
  int  (*getRamSize)();
  void (*pinMode)(int pin, int mode);
  void (*digitalWrite)(int pin, int val);
  int  (*digitalRead)(int pin);
  void (*inputStr)(const char* prompt, char* destBuffer, int maxLen);
  int  (*wifiUp)(const char* ssid, const char* pass);
  void (*wifiDown)();
  int  (*ollamaStream)(const char* prompt, const char* serverIp, const char* modelName, const char* sysPrompt, int streamToConsole);
  void (*getTouch)(TouchState* state);
  int  termWidth;
  int  termHeight;
  int  charWidth;
  int  charHeight;
  void (*drawJpeg)(const char* filename, int x, int y);
  void (*setFKeys)(const char* l1, const char* l2, const char* l3, const char* l4, const char* l5);
  void (*clearFKeys)();
  void* (*malloc)(unsigned int size);
  void (*free)(void* ptr);
  uint32_t (*createSprite)(const char* filename, int spriteSize);
  void (*drawSprite)(uint32_t spriteHandle, int x, int y);
  void (*clearSprite)(uint32_t spriteHandle);
  void (*freeSprite)(uint32_t spriteHandle);
  bool (*initGameMatrix)();
  void (*flushGameMatrix)();
  void (*closeGameMatrix)();
  void (*sysDebugDump)(const char* label, const void* memoryAddress, unsigned int byteCount, uint32_t virtualAddr);
  void (*printAt)(int x, int y, const char* text);
  int  (*random)(int min, int max);
  int  (*serialOpen)(uint32_t baud, int txPin, int rxPin);
  void (*serialWrite)(const uint8_t* buffer, unsigned int length);
  int  (*serialRead)(uint8_t* buffer, unsigned int maxLength);
  void (*serialClose)();
  void* (*sdOpen)(const char* filename, const char* mode);
  uint32_t (*sdRead)(void* fileHandle, void* buffer, uint32_t size, uint32_t count);
  uint32_t (*sdWrite)(void* fileHandle, const void* buffer, uint32_t size, uint32_t count);
  void (*sdClose)(void* filehandle);
} MicroDosAPI;

extern MicroDosAPI kernelAPI;

/* host platform includes */
#ifdef UNIX_HOST
# include <stdint.h>
# include <unistd.h>
#elif defined(WIN32) /*(predefined on MSVC)*/
#elif defined(ESP32_HOST)
#else
# error ***** A platform must be explicitly defined! *****
#endif


/* configurable options */
/* select your host type (or do it in the Makefile):
 #define UNIX_HOST
 #define DEBUGGER
 #define USE_READLINE (defined by default for UNIX_HOST)
 */

#if defined(WIN32) /*(predefined on MSVC)*/
#undef USE_READLINE
#endif

#undef USE_READLINE
/* undocumented, but probably useful */
#undef DEBUG_HEAP
#undef DEBUG_EXPRESSIONS
#undef FANCY_ERROR_MESSAGES
#undef DEBUG_ARRAY_INITIALIZER
#undef DEBUG_LEXER
#undef DEBUG_VAR_SCOPE


#if defined(__hppa__) || defined(__sparc__)
/* the default data type to use for alignment */
#define ALIGN_TYPE double
#else
/* the default data type to use for alignment */
#define ALIGN_TYPE void*
#endif

#define GLOBAL_TABLE_SIZE (97)                /* global variable table */
#define STRING_TABLE_SIZE (97)                /* shared string table size */
#define STRING_LITERAL_TABLE_SIZE (97)        /* string literal table size */
#define RESERVED_WORD_TABLE_SIZE (97)         /* reserved word table size */
#define PARAMETER_MAX (16)                    /* maximum number of parameters to a function */
#define LINEBUFFER_MAX (256)                  /* maximum number of characters on a line */
#define LOCAL_TABLE_SIZE (11)                 /* size of local variable table (can expand) */
#define STRUCT_TABLE_SIZE (11)                /* size of struct/union member table (can expand) */

#define INTERACTIVE_PROMPT_START "Starting PicoC " PICOC_VERSION " ('bye' to exit)\n"
#define INTERACTIVE_PROMPT_STATEMENT "C> "
#define INTERACTIVE_PROMPT_LINE "C> "

extern jmp_buf ExitBuf;
extern jmp_buf HostExitBuf;

extern bool PlatformRunning;

#endif /* PLATFORM_H */
