#include "../interpreter.h"
#include <string.h>

const char Esp32Defs[] =
"struct TouchState { int isPressed; int x; int y; };\n";

extern MicroDosAPI kernelAPI;

void Esp32SetupFunc(Picoc *pc)
{
    VariableDefinePlatformVar(pc, NULL, "INPUT", &pc->IntType, (union AnyValue *)&((int){1}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "OUTPUT", &pc->IntType, (union AnyValue *)&((int){2}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "INPUT_PULLUP", &pc->IntType, (union AnyValue *)&((int){3}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "HIGH", &pc->IntType, (union AnyValue *)&((int){1}), FALSE);
    VariableDefinePlatformVar(pc, NULL, "LOW", &pc->IntType, (union AnyValue *)&((int){0}), FALSE);
}

void Clineno(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    (void)Param; (void)NumArgs;
    ReturnValue->Val->Integer = Parser->Line;
}

/* --- Basic Hardware & Utility Mappings --- */

void CEsp32Beep(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.beep(Param[0]->Val->Integer, Param[1]->Val->Integer);
}

void CEsp32Delay(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.delay(Param[0]->Val->Integer);
}

void CEsp32Peek(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.peek(Param[0]->Val->Integer);
}

void CEsp32Poke(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.poke(Param[0]->Val->Integer, Param[1]->Val->Integer);
}

void CEsp32GetRamSize(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.getRamSize();
}

/* --- GPIO Mappings --- */

void CEsp32PinMode(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.pinMode(Param[0]->Val->Integer, Param[1]->Val->Integer);
}

void CEsp32DigitalWrite(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.digitalWrite(Param[0]->Val->Integer, Param[1]->Val->Integer);
}

void CEsp32DigitalRead(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.digitalRead(Param[0]->Val->Integer);
}

/* --- Wi-Fi & AI Sockets --- */

void CEsp32WifiUp(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.wifiUp(Param[0]->Val->Pointer, Param[1]->Val->Pointer);
}

void CEsp32WifiDown(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.wifiDown();
}

void CEsp32OllamaStream(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.ollamaStream(
        Param[0]->Val->Pointer, // prompt
        Param[1]->Val->Pointer, // serverIp
        Param[2]->Val->Pointer, // modelName
        Param[3]->Val->Pointer, // sysPrompt
        Param[4]->Val->Integer  // streamToConsole
    );
}

/* --- UI / Function Keys --- */

void CEsp32SetFKeys(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.setFKeys(
        Param[0]->Val->Pointer, Param[1]->Val->Pointer,
        Param[2]->Val->Pointer, Param[3]->Val->Pointer, Param[4]->Val->Pointer
    );
}

void CEsp32ClearFKeys(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.clearFKeys();
}

void CEsp32GetTouch(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    void *scriptStructPtr = Param[0]->Val->Pointer;
    if (scriptStructPtr == NULL) return;

    TouchState nativeState = { false, 0, 0 };
    kernelAPI.getTouch(&nativeState);

    int *scriptFields = (int *)scriptStructPtr;

    scriptFields[0] = nativeState.isPressed ? 1 : 0;
    scriptFields[1] = nativeState.x;
    scriptFields[2] = nativeState.y;
}

/* --- Serial Communication --- */

void CEsp32SerialOpen(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.serialOpen(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer);
}

void CEsp32SerialWrite(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    const uint8_t* buffer = (const uint8_t*)Param[0]->Val->Pointer;
    unsigned int length = (unsigned int)Param[1]->Val->Integer;
    kernelAPI.serialWrite(buffer, length);
}

void CEsp32SerialRead(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    uint8_t* buffer = (uint8_t*)Param[0]->Val->Pointer;
    int maxLength = Param[1]->Val->Integer;
    ReturnValue->Val->Integer = kernelAPI.serialRead(buffer, maxLength);
}

/* --- Function Bindings Table --- */
struct LibraryFunction Esp32Functions[] =
{
    { Clineno,              "int lineno();" },
    { CEsp32Beep,           "void beep(int, int);" },
    { CEsp32Delay,          "void delay(int);" },
    { CEsp32Peek,           "int peek(int);" },
    { CEsp32Poke,           "void poke(int, int);" },
    { CEsp32GetRamSize,     "int getRamSize();" },
    { CEsp32PinMode,        "void pinMode(int, int);" },
    { CEsp32DigitalWrite,   "void digitalWrite(int, int);" },
    { CEsp32DigitalRead,    "int digitalRead(int);" },
    { CEsp32WifiUp,         "int wifiUp(char *, char *);" },
    { CEsp32WifiDown,       "void wifiDown();" },
    { CEsp32OllamaStream,   "int ollamaStream(char *, char *, char *, char *, int);" },
    { CEsp32SetFKeys,       "void setFKeys(char *, char *, char *, char *, char *);" },
    { CEsp32ClearFKeys,     "void clearFKeys();" },
    { CEsp32GetTouch,       "void getTouch(struct TouchState *);" },
    { CEsp32SerialOpen,     "int serialOpen(int, int, int);" },
    { CEsp32SerialWrite,    "void serialWrite(char *, int);" },
    { CEsp32SerialRead,     "int serialRead(char *, int);" },
    { NULL,                 NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
    IncludeRegister(pc, "picoc_esp32.h", &Esp32SetupFunc, &Esp32Functions[0], Esp32Defs);
}

