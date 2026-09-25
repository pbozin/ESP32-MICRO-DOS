#include "microdos_api.h"

NEW_STRING(ollama_server, "192.168.0.131");
NEW_STRING(ollama_model,  "qwen2.5-theory-24k:3b");
NEW_STRING(system_prompt, "You are a multidisciplinary philosopher. Keep conversational explanations witty and insightful.");

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;    
    static char promptBuffer[128]; 
    
    delayMs(100); 
    println(STRING("CONNECTING...")); 
    
    if (api->wifiUp(STRING("SSID"), STRING("PASS")) != 0) {
        setColor(RED); 
        println(STRING("ERR: WI-FI PROVISIONING FAILED."));
        setColor(GREEN); 
        return -1;
    }

    delayMs(500);
    int chatting = 1;
    clearScreen(); 
    
    setColor(YELLOW); 
    print(STRING("  --[ "));
    print(ollama_model);
    print(STRING(" ]--[ "));
    print(ollama_server);
    println(STRING(" ]--"));

    while (chatting) {
        setColor(GREEN); 
        api->inputStr(STRING("YOU> "), promptBuffer, sizeof(promptBuffer));
       
        if (strcmp(promptBuffer, STRING("QUIT")) == 0 || strcmp(promptBuffer, STRING("BYE")) == 0) {
            chatting = 0;
            break;
        }
        
        int len = 0;
        while (promptBuffer[len] != '\0') len++;
        if (len == 0) continue;

        setColor(CYAN); 
        print(STRING("AI> "));

        int status = api->ollamaStream(
            promptBuffer, 
            ollama_server, 
            ollama_model,
	    system_prompt,
            1
        );
        
        if (status != 0) {
            setColor(RED); 
            println(STRING("ERR: TRANSMISSION COMPROMISED OR SERVER OFFLINE"));
        }
        println(STRING(""));
    }

    api->wifiDown();
    setColor(GREEN); 
    return 0;
}

