/* graphics.hc */
#include "../interpreter.h"

#ifndef BUILTIN_MINI_STDLIB

static int Graphics_ZeroValue = 0;

void GraphicsGetTermWidth(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.termWidth;
}

void GraphicsGetTermHeight(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.termHeight;
}

void GraphicsGetCharWidth(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.charWidth;
}

void GraphicsGetCharHeight(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.charHeight;
}

void GraphicsSetColor(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.color(Param[0]->Val->Integer);
}

void GraphicsClearScreen(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.clear();
}

void GraphicsCreateSprite(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.createSprite(Param[0]->Val->Pointer, Param[1]->Val->Integer);
}

void GraphicsDrawSprite(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.drawSprite(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer);
}

void GraphicsClearSprite(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.clearSprite(Param[0]->Val->Integer);
}

void GraphicsFreeSprite(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.freeSprite(Param[0]->Val->Integer);
}

void GraphicsInitGameMatrix(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.initGameMatrix();
}

void GraphicsFlushGameMatrix(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.flushGameMatrix();
}

void GraphicsCloseGameMatrix(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.closeGameMatrix();
}

void GraphicsDrawPoint(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.plot(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer);
}

void GraphicsDrawLine(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.line(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer);
}

void GraphicsDrawRect(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.rect(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer);
}

void GraphicsDrawCircle(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.circle(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer);
}

void GraphicsDrawJpeg(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.drawJpeg(Param[0]->Val->Pointer, Param[1]->Val->Integer, Param[2]->Val->Integer);
}

/* all graphics.h functions */
struct LibraryFunction GraphicsFunctions[] =
{
    { GraphicsGetTermWidth,    "int      getTermWidth();" },
    { GraphicsGetTermHeight,   "int      getTermHeight();" },
    { GraphicsGetCharWidth,    "int      getCharWidth();" },
    { GraphicsGetCharHeight,   "int      getCharHeight();" },

    { GraphicsSetColor,        "void     setColor(int colorId);" },
    { GraphicsClearScreen,     "void     clearScreen();" },

    { GraphicsCreateSprite,    "int      createSprite(char* filename, int spriteSize);" },
    { GraphicsDrawSprite,      "void     drawSprite(int spriteHandle, int x, int y);" },
    { GraphicsClearSprite,     "void     clearSprite(int spriteHandle);" },
    { GraphicsFreeSprite,      "void     freeSprite(int spriteHandle);" },

    { GraphicsInitGameMatrix,  "int      initGameMatrix();" },
    { GraphicsFlushGameMatrix, "void     flushGameMatrix();" },
    { GraphicsCloseGameMatrix, "void     closeGameMatrix();" },

    { GraphicsDrawPoint,       "void     drawPoint(int x, int y, int colorId);" },
    { GraphicsDrawLine,        "void     drawLine(int x1, int y1, int x2, int y2, int colorId);" },
    { GraphicsDrawRect,        "void     drawRect(int x, int y, int w, int h, int colorId);" },
    { GraphicsDrawCircle,      "void     drawCircle(int x, int y, int r, int colorId);" },
    { GraphicsDrawJpeg,        "void     drawJpeg(char* filename, int x, int y);" },

    { NULL,             NULL }
};

void GraphicsSetupFunc(Picoc *pc)
{
    if (!VariableDefined(pc, TableStrRegister(pc, "NULL")))
        VariableDefinePlatformVar(pc, NULL, "NULL", &pc->IntType, (union AnyValue *)&Graphics_ZeroValue, FALSE);
}

#endif /* !BUILTIN_MINI_STDLIB */
