#include "../picoc.h"
#include "../interpreter.h"

jmp_buf PicocExitBuf;
jmp_buf HostExitBuf;

bool PlatformRunning = false;

extern MicroDosAPI kernelAPI;

void PlatformInit(Picoc *pc)
{
    (void)pc;
    PlatformRunning = true;
}

void PlatformCleanup(Picoc *pc)
{
    (void)pc;
    PlatformRunning = false;
}

char *PlatformGetLine(char *Buf, int MaxLen, const char *Prompt)
{
    kernelAPI.inputStr(Prompt, Buf, MaxLen);
    return Buf;
}

int PlatformGetCharacter()
{
    return kernelAPI.inkey();
}

void PlatformPutc(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    char buf[2] = { OutCh, '\0' };
    kernelAPI.print(buf);
}

char *PlatformReadFile(Picoc *pc, const char *FileName)
{
    struct stat FileInfo;
    char *ReadText;
    FILE *InFile;
    int BytesRead;
    char *p;

    char fileName[32];

    char* concat(const char* first, const char* second, char* result) {
        char* ptr = result;
        while (*first)  *ptr++ = *first++;
        while (*second) *ptr++ = *second++;
        *ptr = '\0';
        return result;
    }

    concat("/", FileName, fileName);

    if (stat(fileName, &FileInfo))
        ProgramFailNoParser(pc, "can't read file %s\n", fileName);

    ReadText = malloc(FileInfo.st_size + 1);
    if (ReadText == NULL)
        ProgramFailNoParser(pc, "out of memory\n");

    InFile = fopen(fileName, "r");
    if (InFile == NULL)
        ProgramFailNoParser(pc, "can't read file %s\n", fileName);

    BytesRead = fread(ReadText, 1, FileInfo.st_size, InFile);
    if (BytesRead == 0)
        ProgramFailNoParser(pc, "can't read file %s\n", fileName);

    ReadText[BytesRead] = '\0';
    fclose(InFile);

    if ((ReadText[0] == '#') && (ReadText[1] == '!')) {
        for (p = ReadText; (*p != '\0') && (*p != '\r') && (*p != '\n'); ++p) {
            *p = ' ';
        }
    }

    return ReadText;
}

void PicocPlatformScanFile(Picoc *pc, const char *FileName)
{
    char *SourceStr = PlatformReadFile(pc, FileName);
    if (SourceStr != NULL && SourceStr[0] == '#' && SourceStr[1] == '!')
    {
        SourceStr[0] = '/';
        SourceStr[1] = '/';
    }
    PicocParse(pc, FileName, SourceStr, strlen(SourceStr), TRUE, FALSE, TRUE, TRUE);
}

void PlatformExit(Picoc *pc, int RetVal)
{
    if (!pc)
	return;

    pc->PicocExitValue = RetVal;

    if (!PlatformRunning) {
        longjmp(pc->HostExitBuf, RetVal);
    } 
    else if (*pc->PicocExitBuf) {
        longjmp(pc->PicocExitBuf, RetVal);
    }
}
