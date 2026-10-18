#include "microdos_api.h"
#include "microdos_util.h"

NEW_STRING(system_prompt,
    "You are an ESP32 C compiler. "
    "CRITICAL rules:\n"
    "1. You MUST wrap all code inside a standard markdown block starting with ```c and ending with ```\n"
    "2. Keep conversational explanations extremely brief and do not comment the code.\n"
    "3. If there is an error, you will receive it so you can adjust the code.\n"
    "4. You can use the following functions from graphics.h: "
    "enum Color { BLACK, WHITE, LIGHTGREY, RED, ORANGE, YELLOW, GREEN, CYAN, BLUE, MAGENTA, MAROON, DARKGREEN, DARKCYAN, NAVY, PINK, DARKGREY}; "
    "int getTermWidth(); "
    "int getTermHeight(); "
    "int getCharWidth(); "
    "int getCharHeight(); "
    "void setTextColor(int colorId); "
    "void setBgColor(int colorId); "
    "int getTextColor(); "
    "int getBgColor(); "
    "void setKeyboardShift(int state); "
    "int getKeyboardShift(); "
    "void clearScreen(); "
    "void drawPoint(int x, int y, int colorId); "
    "void drawLine(int x1, int y1, int x2, int y2, int colorId); "
    "void drawRect(int x, int y, int w, int h, int colorId); "
    "void fillRect(int x, int y, int w, int h, int colorId); "
    "void drawCircle(int x, int y, int r, int colorId); "
    "void fillCircle(int x, int y, int r, int colorId); "
    "void drawJpeg(char* filename, int x, int y); "
    "int createSprite(char* filename, int spriteSize); "
    "void drawSprite(int spriteHandle, int x, int y, int frame); "
    "void clearSprite(int spriteHandle); "
    "void freeSprite(int spriteHandle); "
    "int initGameMatrix(); "
    "void flushGameMatrix(); "
    "void closeGameMatrix();\n"
    "5. You can use the following functions from picoc_esp32.h: "
    "void beep(int, int); "
    "void delay(int); "
    "int peek(int); "
    "void poke(int, int); "
    "int getRamSize(); "
    "void pinMode(int, int); "
    "void digitalWrite(int, int); "
    "int digitalRead(int); "
    "int wifiUp(char *, char *); "
    "void wifiDown(); "
    "int ollamaStream(char *, char *, char *, char *, char*, int, int); "
    "void setFKeys(char *, char *, char *, char *, char *); "
    "void clearFKeys(); "
    "void getTouch(struct TouchState *); "
    "int serialOpen(int, int, int); "
    "void serialWrite(char *, int); "
    "int serialRead(char *, int);\n"
    "6. You can use the standard functions from stdio.h, stdlib.h, fcnlt.h, math.h.\n"
    "7. You should use getchar() before exiting or calling closeGameMatrix() so user can examine outcome.\n"
    "8. You should use initGameMatrix() only if using sprites or nonblocking getchar() loop.\n"
    "9. If using initGameMtrix(), you should always use flushGameMatrix() after drawing to the display, but use it sparingly.\n"
    "10. Do not comment the code.");

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    static char ALIGNED promptBuffer[128];
    static char ALIGNED errorBuffer[256];

    delay(100);
    termPrintln(STRING("CONNECTING..."));

    if (wifiUp(STRING("CFG"), STRING("CFG")) != 0) {
        termPrintln(STRING("ERR: WI-FI PROVISIONING FAILED."));
        return -1;
    }

    delay(500);

    int origTextColor = getTextColor();
    int origBgColor = getBgColor();
    int origKeyboardShift = getKeyboardShift();

    setKeyboardShift(0);
    setBgColor(BLACK);
    setTextColor(GREEN);

    clearScreen();

    int chatting = 1;
    while (chatting) {
        setTextColor(GREEN);
        inputStr(STRING("YOU> "), promptBuffer, sizeof(promptBuffer));

        if (strcmp(promptBuffer, STRING("QUIT")) == 0 || strcmp(promptBuffer, STRING("BYE")) == 0 ||
            strcmp(promptBuffer, STRING("quit")) == 0 || strcmp(promptBuffer, STRING("bye")) == 0 ) {
            chatting = 0;
            break;
        }

        int len = 0;
        while (promptBuffer[len] != '\0') len++;
        if (len == 0) continue;

        setTextColor(CYAN);
        termPrint(STRING("AI> "));

        int status = ollamaStream(
            promptBuffer,
            STRING("CFG"),
            STRING("CFG"),
	    system_prompt,
	    errorBuffer,
	    sizeof(errorBuffer),
            1
        );

        if (status != 0) {
            setTextColor(RED);
            termPrintln(STRING("ERR: SERVER OFFLINE"));
        }
        termPrintln(STRING(""));
    }

    wifiDown();
    setKeyboardShift(origKeyboardShift);
    setBgColor(origBgColor);
    setTextColor(origTextColor);
    return 0;
}

