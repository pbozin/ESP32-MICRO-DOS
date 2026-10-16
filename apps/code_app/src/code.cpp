#include "microdos_api.h"
#include "microdos_util.h"

NEW_STRING(system_prompt, "You are an ESP32 BASIC compiler. "
                          "CRITICAL rules:\n"
                          "1. You MUST wrap all code inside a standard markdown block starting with ```basic and ending with ```\n"
                          "2. Every single line of BASIC code MUST have a line number.\n"
                          "3. NEVER put multiple line numbers on the same line. EVERY line number must start on a brand new newline (\\n).\n"
                          "Example:\n10 PRINT \"HI\"\n20 GOTO 10\n"
                          "Keep conversational explanations extremely brief.\n");

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    static char promptBuffer[128];
    static char errorBuffer[64];

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

    setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));

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
    setTextColor(origTextColor);
    setBgColor(origBgColor);
    return 0;
}

