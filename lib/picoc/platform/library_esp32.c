
#include "../interpreter.h"

void Esp32SetupFunc(Picoc *pc)
{
    (void)pc;
}

void Clineno(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    (void)Param;
    (void)NumArgs;
    ReturnValue->Val->Integer = Parser->Line;
}

struct LibraryFunction Esp32Functions[] =
{
    { Clineno,      "int lineno();" },
    { NULL,         NULL }
};

void PlatformLibraryInit(Picoc *pc)
{
    IncludeRegister(pc, "picoc_esp32.h", &Esp32SetupFunc, &Esp32Functions[0], NULL);
}
