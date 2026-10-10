#include "../picoc.h"
#include "../interpreter.h"
#include <sys/stat.h>

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
    int ch = kernelAPI.inkey();
    // Return standard C EOF if no character is currently waiting in the buffer
    if (ch <= 0) {
        return -1;
    }
    return ch;
}

void PlatformPutc(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    (void)Stream;
    char buf[2] = { (char)OutCh, '\0' };
    kernelAPI.print(buf);
}

char *PlatformReadFile(Picoc *pc, const char *FileName)
{
    struct stat FileInfo;
    char *ReadText;
    FILE *InFile;
    int BytesRead;
    char *p;

    char *fileName = (char *)malloc(32);

    if (FileName[0] == '/') {
        snprintf(fileName, 32, "/sd%s", FileName);
    } else {
        snprintf(fileName, 32, "/sd/%s", FileName);
    }

    if (stat(fileName, &FileInfo))
        ProgramFailNoParser(pc, "can't read file %s\n", fileName);

    ReadText = malloc(FileInfo.st_size + 1);
    if (ReadText == NULL)
        ProgramFailNoParser(pc, "out of memory\n");

    InFile = fopen(fileName, "r");
    if (InFile == NULL) {
        free(ReadText);
        ProgramFailNoParser(pc, "can't read file\n", fileName);
    }

    BytesRead = fread(ReadText, 1, FileInfo.st_size, InFile);
    fclose(InFile);
    free(fileName);

    if (BytesRead <= 0) {
        free(ReadText);
        ProgramFailNoParser(pc, "error reading file\n");
    }

    ReadText[BytesRead] = '\0';

    // Shebang line handling (e.g. #!/usr/bin/picoc) -> Convert to white spaces
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
    if (SourceStr != NULL)
    {
        PicocParse(pc, FileName, SourceStr, strlen(SourceStr), TRUE, FALSE, TRUE, TRUE);
        free(SourceStr); // Fix: Essential to free heap string block after PicoC completes parsing it!
    }
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
