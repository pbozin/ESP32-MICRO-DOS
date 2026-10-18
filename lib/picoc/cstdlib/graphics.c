/* graphics.hc */
#include "../interpreter.h"

#ifndef BUILTIN_MINI_STDLIB

const char GraphicsDefs[] =
"#define BLACK      0\n"
"#define WHITE      1\n"
"#define LIGHTGREY  2\n"
"#define RED        3\n"
"#define ORANGE     4\n"
"#define YELLOW     5\n"
"#define GREEN      6\n"
"#define CYAN       7\n"
"#define BLUE       8\n"
"#define MAGENTA    9\n"
"#define MAROON     10\n"
"#define DARKGREEN  11\n"
"#define DARKCYAN   12\n"
"#define NAVY       13\n"
"#define PINK       14\n"
"#define DARKGREY   15\n";

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

void GraphicsSetTextColor(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.setTextColor(Param[0]->Val->Integer);
}

void GraphicsSetBgColor(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.setBgColor(Param[0]->Val->Integer);
}

void GraphicsGetBgColor(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.getBgColor();
}

void GraphicsGetTextColor(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.getTextColor();
}

void GraphicsGetKeyboardShift(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.getKeyboardShift();
}

void GraphicsSetKeyboardShift(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.setKeyboardShift(Param[0]->Val->Integer);
}

void GraphicsPrintAt(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.printAt(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Pointer);
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
    kernelAPI.drawSprite(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer);
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
    kernelAPI.drawLine(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer);
}

void GraphicsDrawRect(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.drawRect(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer);
}

void GraphicsDrawCircle(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.drawCircle(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer);
}

void GraphicsFillRect(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.fillRect(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer);
}

void GraphicsFillCircle(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.fillCircle(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer);
}

void GraphicsDrawJpeg(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    kernelAPI.drawJpeg(Param[0]->Val->Pointer, Param[1]->Val->Integer, Param[2]->Val->Integer);
}

void GraphicsCreateCollisionMap(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
    ReturnValue->Val->Integer = kernelAPI.createCollisionMap(Param[0]->Val->Integer, Param[1]->Val->Integer, Param[2]->Val->Integer, Param[3]->Val->Integer, Param[4]->Val->Integer, Param[5]->Val->Integer, Param[6]->Val->Integer, Param[7]->Val->Pointer);
}

/* all graphics.h functions */
struct LibraryFunction GraphicsFunctions[] =
{
    { GraphicsGetTermWidth,    "int      getTermWidth();" },
    { GraphicsGetTermHeight,   "int      getTermHeight();" },
    { GraphicsGetCharWidth,    "int      getCharWidth();" },
    { GraphicsGetCharHeight,   "int      getCharHeight();" },

    { GraphicsSetTextColor,    "void     setTextColor(int colorId);" },
    { GraphicsSetBgColor,      "void     setBgColor(int colorId);" },
    { GraphicsGetTextColor,    "int      getTextColor();" },
    { GraphicsGetBgColor,      "int      getBgColor();" },
    { GraphicsSetKeyboardShift,"void     setKeyboardShift(int state);" },
    { GraphicsGetKeyboardShift,"int      getKeyboardShift();" },
    { GraphicsClearScreen,     "void     clearScreen();" },

    { GraphicsPrintAt,         "void     printAt(int x, int y, char* text);" },

    { GraphicsDrawPoint,       "void     drawPoint(int x, int y, int colorId);" },
    { GraphicsDrawLine,        "void     drawLine(int x1, int y1, int x2, int y2, int colorId);" },
    { GraphicsDrawRect,        "void     drawRect(int x, int y, int w, int h, int colorId);" },
    { GraphicsFillRect,        "void     fillRect(int x, int y, int w, int h, int colorId);" },
    { GraphicsDrawCircle,      "void     drawCircle(int x, int y, int r, int colorId);" },
    { GraphicsFillCircle,      "void     fillCircle(int x, int y, int r, int colorId);" },
    { GraphicsDrawJpeg,        "void     drawJpeg(char* filename, int x, int y);" },

    { GraphicsCreateSprite,    "int      createSprite(char* filename, int spriteSize);" },
    { GraphicsDrawSprite,      "void     drawSprite(int spriteHandle, int x, int y, int color, int frame);" },
    { GraphicsClearSprite,     "void     clearSprite(int spriteHandle);" },
    { GraphicsFreeSprite,      "void     freeSprite(int spriteHandle);" },

    { GraphicsInitGameMatrix,  "int      initGameMatrix();" },
    { GraphicsFlushGameMatrix, "void     flushGameMatrix();" },
    { GraphicsCloseGameMatrix, "void     closeGameMatrix();" },
    { GraphicsCreateCollisionMap,"int    createCollisionMap(int x, int y, int w, int h, int color, int finalCols, int finalRows, int* map);" },

    { NULL,                    NULL }
};

void GraphicsSetupFunc(Picoc *pc)
{
}

#endif /* !BUILTIN_MINI_STDLIB */
