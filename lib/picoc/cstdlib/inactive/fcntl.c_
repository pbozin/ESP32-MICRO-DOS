#include "../interpreter.h"
#include <fcntl.h>

const char FcntlDefs[] =
"#define O_RDONLY   0\n"
"#define O_WRONLY   1\n"
"#define O_RDWR     2\n"
"#define O_CREAT    0x0040\n"
"#define O_EXCL     0x0800\n"
"#define O_TRUNC    0x0200\n"
"#define O_APPEND   0x0400\n"
"#define O_SYNC     0x2000\n"
"#define O_NONBLOCK 0x4000\n"
"#define O_NOCTTY   0x8000\n";

void CFcntlOpen(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    (void)Parser;

    const char *Path = Param[0]->Val->Pointer;
    int Flags = Param[1]->Val->Integer;
    int Mode = (NumArgs > 2) ? Param[2]->Val->Integer : 0;

    ReturnValue->Val->Integer = open(Path, Flags, Mode);
}

struct LibraryFunction FcntlFunctions[] =
{
    { CFcntlOpen, "int open(char *, int, int);" },
    { NULL, NULL }
};

void FcntlSetupFunc(Picoc *pc)
{
    (void)pc;
}