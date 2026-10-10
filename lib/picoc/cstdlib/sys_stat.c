#include "../interpreter.h"
#include <sys/stat.h>

// Custom simplified Stat
const char StatDefs[] =
    "struct stat {\n"
    "    int st_size;   /* file size in bytes */\n"
    "    int st_mode;   /* raw mode bits*/\n"
    "    int is_dir;    /* 1 if directory, 0 otherwise */\n"
    "    int is_file;   /* 1 if file, 0 otherwise */\n"
    "};\n";

void CStat(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    (void)Parser;
    (void)NumArgs;

    const char *Path = Param[0]->Val->Pointer;
    void *StructAddr = Param[1]->Val->Pointer; 
 
    struct stat RealInfo;
    int Result = stat(Path, &RealInfo);

    if (Result == 0)
    {
        int *Fields = (int *)StructAddr;

        Fields[0] = (int)RealInfo.st_size;
        Fields[1] = (int)RealInfo.st_mode;
        Fields[2] = S_ISDIR(RealInfo.st_mode) ? 1 : 0;
        Fields[3] = S_ISREG(RealInfo.st_mode) ? 1 : 0;
    }

    ReturnValue->Val->Integer = Result;
}

void CMkDir(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    (void)Parser;
    (void)NumArgs;
    const char *Path = Param[0]->Val->Pointer;
    mode_t Mode = (mode_t)Param[1]->Val->Integer;
    ReturnValue->Val->Integer = mkdir(Path, Mode);
}

struct LibraryFunction StatFunctions[] =
{
    { CStat, "int stat(char *, struct stat *);" },
    { CMkDir, "int mkdir(char *, int);"},
    { NULL,  NULL }
};

void StatSetupFunc(Picoc *pc)
{
    (void)pc;
}