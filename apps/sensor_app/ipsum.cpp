#include "microdos_api.h" 

static MicroDosAPI* os ALIGNED = NULL;

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    os = api;

    char output[50]; 
    char* testString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

    os->println(STRING("--- STARTING MEMCPY BOUNDARY TEST ---"));

    for (int a = 0; a < 50; a++) {
        for(int i = 0; i < 50; i++) output[i] = '?';

        memcpy(output, testString, a);

        output[a] = '\0';

        os->println(output);
    }

    os->println(STRING("--- MEMCPY TEST COMPLETE ---"));
    return 0;
}
