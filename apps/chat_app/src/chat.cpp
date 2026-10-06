#include "microdos_api.h"
#include "microdos_util.h"

NEW_STRING(ollama_server, "192.168.0.131");
NEW_STRING(ollama_model,  "qwen2.5-theory-24k:3b");
NEW_STRING(system_prompt, "You are a multidisciplinary philosopher. Keep conversational explanations witty and insightful.\n");

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    static char promptBuffer[128];

    delayMs(100);
    termPrintln(STRING("CONNECTING..."));

    if (wifiUp(STRING("SSID"), STRING("PASS")) != 0) {
        setColor(RED);
        termPrintln(STRING("ERR: WI-FI PROVISIONING FAILED."));
        setColor(GREEN);
        return -1;
    }

    delayMs(500);
    int chatting = 1;
    termClear();

    setColor(YELLOW);
    termPrint(STRING("  --[ "));
    if (getTermWidth() < 320) {
        termPrint(ollama_model);
    } else {
        termPrint(ollama_model);
        termPrint(STRING(" ]--[ "));
        termPrint(ollama_server);
    }
    termPrintln(STRING(" ]--"));

    while (chatting) {
        setColor(GREEN);
        inputStr(STRING("YOU> "), promptBuffer, sizeof(promptBuffer));

        if (strcmp(promptBuffer, STRING("QUIT")) == 0 || strcmp(promptBuffer, STRING("BYE")) == 0) {
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
            ollama_server,
            ollama_model,
	    system_prompt,
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

