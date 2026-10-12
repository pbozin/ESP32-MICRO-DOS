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

    delay(100);
    termPrintln(STRING("CONNECTING..."));

    if (wifiUp(STRING("CFG"), STRING("CFG")) != 0) {
        setColor(RED);
        termPrintln(STRING("ERR: WI-FI PROVISIONING FAILED."));
        setColor(GREEN);
        return -1;
    }

    delay(500);
    int chatting = 1;
    clearScreen();

    while (chatting) {
        setColor(GREEN);
        inputStr(STRING("YOU> "), promptBuffer, sizeof(promptBuffer));

        if (strcmp(promptBuffer, STRING("QUIT")) == 0 || strcmp(promptBuffer, STRING("BYE")) == 0 ||
            strcmp(promptBuffer, STRING("quit")) == 0 || strcmp(promptBuffer, STRING("bye")) == 0 ) {
            chatting = 0;
            break;
        }

        int len = 0;
        while (promptBuffer[len] != '\0') len++;
        if (len == 0) continue;

        setColor(CYAN);
        termPrint(STRING("AI> "));

        int status = ollamaStream(
            promptBuffer,
            STRING("CFG"),
            STRING("CFG"),
	    system_prompt,
	    NULL,
	    0,
            1
        );

        if (status != 0) {
            setColor(RED);
            termPrintln(STRING("ERR: SERVER OFFLINE"));
        }
        termPrintln(STRING(""));
    }

    wifiDown();
    setColor(GREEN);
    return 0;
}

