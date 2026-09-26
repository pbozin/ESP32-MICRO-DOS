#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <FS.h>
#include <TJpg_Decoder.h>
#include <string.h>


#if defined(BOARD_CYD)
#include <SD.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
SPIClass sdSPI(HSPI);

#elif defined(BOARD_JC3248)
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <SD_MMC.h>
#define SD SD_MMC
#define TFT_BLACK     0x0000
#define TFT_BLUE      0x001F
#define TFT_RED       0xF800
#define TFT_GREEN     0x07E0
#define TFT_CYAN      0x07FF
#define TFT_MAGENTA   0xF81F
#define TFT_YELLOW    0xFFE0
#define TFT_WHITE     0xFFFF
#define TFT_DARKGREY  0x7BEF
#define TFT_LIGHTGREY 0xD69A
#define TFT_ORANGE    0xFD20
#define TFT_MAROON    0x8000
#define TFT_DARKGREEN 0x03E0
#define TFT_DARKCYAN  0x03EF
#define TFT_NAVY      0x000F
#define TFT_PINK      0xFE19
#endif


static uint16_t currentActivePaletteId = TFT_GREEN;

#if defined(BOARD_JC3248)
  static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    45 /* CS */, 47 /* SCK */, 21 /* D0 */, 48 /* D1 */, 40 /* D2 */, 39 /* D3 */
  );

  static Arduino_GFX *tft_base = new Arduino_AXS15231B(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* Rotation */, false /* IPS */, TFT_WIDTH, TFT_HEIGHT
  );

  static Arduino_Canvas *canvas_bridge = new Arduino_Canvas(
    TFT_WIDTH, TFT_HEIGHT, tft_base, 0 /* output_x */, 0 /* output_y */, 0 /* rotation */
  );

  static Arduino_GFX *tft_driver = canvas_bridge;

  class GFX_CompatibilityWrapper {
  public:
    void init() {

      tft_driver->begin();
      tft_driver->fillScreen(TFT_BLACK);

      pinMode(3, INPUT);

      Wire.begin(4 /* SDA */, 8 /* SCL */);
      Wire.setClock(400000);

      static const uint8_t AXS_READ_TOUCHPAD[8] = { 0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08 };
      Wire.beginTransmission(0x3B);
      Wire.write(AXS_READ_TOUCHPAD, 8);
      Wire.endTransmission();
      delayMicroseconds(50);
    }
    void initDMA() {}
    void setRotation(uint8_t r) { tft_driver->setRotation(r); }
    void fillScreen(uint16_t color) { tft_driver->fillScreen(color); canvas_bridge->flush(); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) { tft_driver->fillRect(x,y,w,h,c);}
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t c) { tft_driver->drawRect(x,y,w,h,c);}
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t c) { tft_driver->drawLine(x0,y0,x1,y1,c);}
    void drawPixel(int16_t x, int16_t y, uint16_t c) { tft_driver->drawPixel(x,y,c); }
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t c) { tft_driver->fillCircle(x, y, r, c);}
    void drawChar(char c, int16_t x, int16_t y) { tft_driver->drawChar(x, y, (unsigned char)c, currentActivePaletteId, TFT_BLACK);}

    void setTextColor(uint16_t c) { tft_driver->setTextColor(c, TFT_BLACK); }
    void setTextColor(uint16_t c, uint16_t b) { tft_driver->setTextColor(c, b); }

    void setCursor(int16_t x, int16_t y) { tft_driver->setCursor(x, y); }

    void print(const char* str) { tft_driver->print(str); }
    void print(char c) { tft_driver->print(c); }

    void setTextFont(uint8_t f) { }
    void setTextSize(uint8_t s) { tft_driver->setTextSize(s); }
    int16_t textWidth(const char* str) { return strlen(str) * 6; }
    void drawString(const char* str, int16_t x, int16_t y) {
      tft_driver->setCursor(x, y);
      tft_driver->print(str);
    }

    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t c) { tft_driver->drawFastHLine(x, y, w, c);}
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t c) { tft_driver->drawFastVLine(x, y, h, c);}
    int16_t width() { return TFT_WIDTH; }
    int16_t height() { return TFT_HEIGHT; }
    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* bitmap) {
      tft_driver->draw16bitRGBBitmap(x, y, bitmap, w, h);
      canvas_bridge->flush();
    }
    bool getTouch(uint16_t *x, uint16_t *y) {
      Wire.requestFrom(0x3B, 8);
      if (Wire.available() >= 8) {
        uint8_t packet[8];
        for (int i = 0; i < 8; i++) {
          packet[i] = Wire.read();
        }
        if (packet[0] != 0x00 || packet[1] == 0x00) return false;

        uint16_t rawX = ((uint16_t)(packet[2] & 0x0F) << 8) | packet[3];
        uint16_t rawY = ((uint16_t)(packet[4] & 0x0F) << 8) | packet[5];

        if (rawX < TFT_WIDTH && rawY < TFT_HEIGHT) {
          *x = rawX;
          *y = rawY;
	  delay(33);
          return true;
        }
      }
      return false;
    }
  };
  static GFX_CompatibilityWrapper tft;

  class TFT_eSprite {
  private:
    int16_t _w = 0;
    int16_t _h = 0;
    uint8_t* _fb = nullptr;
    uint16_t* _palette = nullptr;

  public:
    TFT_eSprite(void* tft_ptr) {}
    ~TFT_eSprite() { deleteSprite(); }

    uint8_t* frameBuffer(uint8_t component) { return _fb; }
    int16_t width()  { return _w; }
    int16_t height() { return _h; }
    void setColorDepth(uint8_t d) { }

    void createSprite(int16_t w, int16_t h) {
      deleteSprite();
      _w = w; _h = h;
      int allocationSize = (_w * _h) / 2;
      _fb = (uint8_t*)malloc(allocationSize);
      if (_fb) memset(_fb, 0, allocationSize);
    }

    void deleteSprite() {
      if (_fb) { free(_fb); _fb = nullptr; }
      _w = 0; _h = 0;
    }

    void createPalette(uint16_t* paletteArray) { _palette = paletteArray; }

    void fillSprite(uint16_t colorIndex) {
      if (!_fb) return;
      uint8_t packedByte = ((colorIndex & 0x0F) << 4) | (colorIndex & 0x0F);
      memset(_fb, packedByte, (_w * _h) / 2);
    }

    void drawPixel(int16_t x, int16_t y, uint16_t colorIndex) {
      if (!_fb || x < 0 || x >= _w || y < 0 || y >= _h) return;
      int globalPixelIdx = (y * _w) + x;
      int byteIdx = globalPixelIdx >> 1;
      if ((globalPixelIdx & 1) == 0) {
        _fb[byteIdx] = (_fb[byteIdx] & 0x0F) | ((colorIndex & 0x0F) << 4);
      } else {
        _fb[byteIdx] = (_fb[byteIdx] & 0xF0) | (colorIndex & 0x0F);
      }
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colorIndex) {
      for (int16_t j = y; j < y + h; j++) {
        for (int16_t i = x; i < x + w; i++) {
          drawPixel(i, j, colorIndex);
        }
      }
    }

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t colorIndex) {
      int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
      int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
      int16_t err = dx + dy, e2;
      while (true) {
        drawPixel(x0, y0, colorIndex);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
      }
    }

    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t colorIndex) {
      for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
          if (x*x + y*y <= r*r) {
            drawPixel(x0 + x, y0 + y, colorIndex);
          }
        }
      }
    }

    void print(const char* str) { tft_driver->print(str); }
    void print(char c) { tft_driver->print(c); }

    void pushImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t* bitmap) {
      if (!_fb || !_palette) return;
      for (int16_t row = 0; row < h; row++) {
        for (int16_t col = 0; col < w; col++) {
          uint16_t color = bitmap[row * w + col];
          uint8_t closestIndex = 0;
          drawPixel(x + col, y + row, closestIndex);
        }
      }
    }

    void pushSprite(int16_t x, int16_t y) {
      if (!_fb || !_palette) return;
      uint16_t rowBuffer[TFT_WIDTH];
      int16_t targetWidth = (_w > TFT_WIDTH) ? TFT_WIDTH : _w;
      int16_t targetHeight = (_h > TFT_WIDTH) ? TFT_WIDTH : _h;

      for (int16_t row = 0; row < targetHeight; row++) {
        int canvasRowOffset = row * (_w / 2);
        for (int16_t col = 0; col < targetWidth; col++) {
          int byteIdx = canvasRowOffset + (col >> 1);
          uint8_t rawByte = _fb[byteIdx];
          uint8_t paletteIndex = ((col & 1) == 0) ? ((rawByte >> 4) & 0x0F) : (rawByte & 0x0F);
          rowBuffer[col] = _palette[paletteIndex];
        }
        tft_driver->draw16bitRGBBitmap(x, y + row, rowBuffer, targetWidth, 1);
      }
    }

    void setTextColor(uint16_t c, uint16_t b) { tft_driver->setTextColor(c, b); }
    void setCursor(int16_t x, int16_t y) { tft_driver->setCursor(x, y); }
    int16_t textWidth(const char* str) { return strlen(str) * 6; }
  };

#endif

bool matchFuncBounds(const char* str, const char* prefix, int prefixLen, int &innerLen);
bool runMultiStatementLine(const char* fullLine, int &nextIdx);
bool runSingleLine(const char* rawLine, int &currentLineIdx);
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap);
char getKeyPress(bool blocking);
const char* getStringVariable(const char* name);
int evaluateExpression(const char* rawExpr);
int getVariable(const char* name);
uint16_t getPaletteColor(int cId);
void api_setup();
void appendToBuffer(char* dest, const char* src, size_t destMaxSize);
void clearConversationContext();
void clearProgram();
void drawKeyboard();
void drawStatusBar();
void executeProgram();
void handleKeyPress(char key);
void initSD();
void initTouchCalibration();
void listProgram();
void listProgramRange(int start, int end);
void loadBuffer(const char* filename);
void loadFile(const char* filename);
void loadJpegToScreen(const char* filename, int16_t xOut, int16_t yOut);
void loop();
void nativeBinaryTaskWorker(void* pvParameters);
void playBeep(int frequency, int durationMs);
void printLogo();
void processCommand(const char* cmd);
void processCommand(const char* rawCmd);
void processTerminalTouchScrolling();
void redrawTerminal();
void renderFullWrappedInputLine();
void saveBuffer(const char* filename);
void saveFile(const char* filename);
void setStringVariable(const char* name, const char* val);
void setup();
void setVariable(const char* name, int val);
void slideWindowAppend(char* dest, const char* src, size_t maxWindowSize);
void storeLine(int num, const char* codeLine);
void stripQuotes(char* str);
void terminalPrint(const char* text);
void terminalPrintln(const char* text = "");
void terminalPrintln(const char* text);
void terminalScrollDaemonTask(void* pvParameters);
void trimCString(char* str);
void writeEscapedToStream(WiFiClient* stream, const char* src);
struct TouchState { bool isPressed; int x; int y; };
void kernel_getTouchState(TouchState* state);
void decompressRLEToCanvas(TFT_eSprite* canvas, int startX, int startY, const uint8_t* rleData, int rleSize);
int compress4BitRLE(const uint8_t* source, uint8_t* destination);
void processIncomingToken(const char* token, const char* m, bool &isRecordingCode, char* textAccumulator,
	         	  size_t textAccumMaxSize, char* markdownBuffer, size_t mdMaxSize, bool &insideThinkBlock,
			  char* fullAssistantResponse, size_t farMaxSize, int streamToConsole);


const uint16_t ramOSPalette[16] = {
    TFT_BLACK,      // Index 0
    TFT_WHITE,      // Index 1
    TFT_LIGHTGREY,  // Index 2
    TFT_RED,        // Index 3
    TFT_ORANGE,     // Index 4
    TFT_YELLOW,     // Index 5
    TFT_GREEN,      // Index 6
    TFT_CYAN,       // Index 7
    TFT_BLUE,       // Index 8
    TFT_MAGENTA,    // Index 9
    TFT_MAROON,     // Index 10
    TFT_DARKGREEN,  // Index 11
    TFT_DARKCYAN,   // Index 12
    TFT_NAVY,       // Index 13
    TFT_PINK,       // Index 14
    TFT_DARKGREY    // Index 15  <- Chroma key or dark grey
};

uint16_t getPaletteColor(int cId) {
  switch (cId) {
    case 0: return TFT_BLACK;
    case 1: return TFT_WHITE;
    case 2: return TFT_LIGHTGREY;
    case 3: return TFT_RED;
    case 4: return TFT_ORANGE;
    case 5: return TFT_YELLOW;
    case 6: return TFT_GREEN;
    case 7: return TFT_CYAN;
    case 8: return TFT_BLUE;
    case 9: return TFT_MAGENTA;
    case 10: return TFT_MAROON;
    case 11: return TFT_DARKGREEN;
    case 12: return TFT_DARKCYAN;
    case 13: return TFT_NAVY;
    case 14: return TFT_PINK;
    case 15: return TFT_DARKGREY; // Chroma key or dark grey

    default: return TFT_GREEN;
  }
}

#define SYSTEM_RAM_SIZE 165
int systemRAM[SYSTEM_RAM_SIZE];

#define CONTEXT_BUF_SIZE 2048
static char coderContext[CONTEXT_BUF_SIZE] = "";
static char chatContext[CONTEXT_BUF_SIZE] = "";
static char* activeContext = NULL;

#define CHAR_WIDTH  6
#define CHAR_HEIGHT 16
#define TERM_COLS   int(TFT_WIDTH / CHAR_WIDTH)
#define TERM_ROWS   int(TFT_WIDTH / CHAR_HEIGHT)

#define CURSOR_SIZE 2
#define FILENAME_SIZE 20
#define PROMPT_SIZE 40
#define INPUT_BUF_SIZE (TERM_COLS * 2)

static int inputCursorPos = 0;

static char inputBuffer[INPUT_BUF_SIZE] = "";
static char cursor[CURSOR_SIZE] = "_";
static char previousPrompt[PROMPT_SIZE] = ": ";
static char currentPrompt[PROMPT_SIZE] = "> ";
static char logBuf[TERM_COLS];
static char errBuf[TERM_COLS];
static char baseFilename[FILENAME_SIZE];
static char fixedFilename[FILENAME_SIZE];

static bool programHalted = false;

#define TKN_F1 '\x11'
#define TKN_F2 '\x12'
#define TKN_F3 '\x13'
#define TKN_F4 '\x14'
#define TKN_F5 '\x15'

static const char fTable[5] = { TKN_F1, TKN_F2, TKN_F3, TKN_F4, TKN_F5 };

#define F_KEY_LABEL_SIZE 7
static const char fKeyLabelsDefault[5][F_KEY_LABEL_SIZE] = { " left ", " rght ", "  up  ", " down ", " bksp " };
static char fKeyLabels[5][F_KEY_LABEL_SIZE] = { "  <-  ", "  ->  ", "      ", "      ", "  <X  " };

static bool fKeysOverlayActive = false;

#define KEY_ROWS   4
#define KEY_COLS   10
#define KEY_HEIGHT 34
#define KEY_WIDTH  32

#define STATUS_HEIGHT   24
#define STATUS_Y_START  (TFT_HEIGHT - (KEY_ROWS*KEY_HEIGHT) - STATUS_HEIGHT)

#define KEYBOARD_Y_START (TFT_HEIGHT - (KEY_ROWS*KEY_HEIGHT))

#define TOTAL_ROWS TERM_ROWS * 4  // Total capacity of historical terminal memory

static char terminalBuffer[TOTAL_ROWS][TERM_COLS + 1];
static uint8_t colorBuffer[TOTAL_ROWS][TERM_COLS];

static int scrollOffset = 0;
static int activeRowIndex = 0;

static bool symbolModeActive = false;

#define SPRITE_SIZE 40
#define COLOR_DEPTH 4

static TFT_eSprite bgCanvas = TFT_eSprite(&tft);
static TFT_eSprite renderSprite = TFT_eSprite(&tft);
static TFT_eSprite tempSprite = TFT_eSprite(&tft);

#define MATRIX_ACTIVE (bgCanvas.frameBuffer(0) != nullptr)
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

#define COMPRESSED_4BIT_SIZE 400
#define PACKED_BYTES_PER_SPRITE ((SPRITE_SIZE * SPRITE_SIZE) / (8 / COLOR_DEPTH))

struct OSSprite_t {
    bool isOnScreen;
    uint8_t rawSprite[COMPRESSED_4BIT_SIZE];
    size_t rawSize;
    uint8_t backupSprite[COMPRESSED_4BIT_SIZE];
    size_t backupSize;
    int lastX, lastY;
};

int compress4BitRLE(const uint8_t* source, uint8_t* destination) {
    int srcIdx = 0;
    int destIdx = 0;
    int totalPixels = SPRITE_SIZE * SPRITE_SIZE;

    while (srcIdx < totalPixels) {
        uint8_t currentColor = (srcIdx % 2 == 0) ? (source[srcIdx / 2] >> 4) : (source[srcIdx / 2] & 0x0F);
        uint8_t runLength = 1;
        srcIdx++;

        while (srcIdx < totalPixels && runLength < 15) {
            uint8_t nextColor = (srcIdx % 2 == 0) ? (source[srcIdx / 2] >> 4) : (source[srcIdx / 2] & 0x0F);
            if (nextColor == currentColor) {
                runLength++;
                srcIdx++;
            } else {
                break;
            }
        }
        destination[destIdx++] = (currentColor << 4) | (runLength & 0x0F);
    }
    return destIdx;
}

void decompressRLEToCanvas(TFT_eSprite* canvas, int startX, int startY, const uint8_t* rleData, int rleSize) {
    uint8_t* buffer = (uint8_t*)canvas->frameBuffer(0);
    if (!buffer) return;

    int canvasWidth = canvas->width();
    int pixelCount = 0;

    for (int i = 0; i < rleSize; i++) {
        uint8_t color = rleData[i] >> 4;
        uint8_t run   = rleData[i] & 0x0F;

        for (int r = 0; r < run; r++) {
            int localX = pixelCount % SPRITE_SIZE;
            int localY = pixelCount / SPRITE_SIZE;
            pixelCount++;

            int targetX = startX + localX;
            int targetY = startY + localY;

            if (targetX < 0 || targetX >= canvasWidth || targetY < 0 || targetY >= canvas->height()) {
                continue;
            }

            int globalPixelIdx = (targetY * canvasWidth) + targetX;
            int byteIdx = globalPixelIdx >> 1;

            if ((globalPixelIdx & 1) == 0) {
                buffer[byteIdx] = (buffer[byteIdx] & 0x0F) | (color << 4);
            } else {
                buffer[byteIdx] = (buffer[byteIdx] & 0xF0) | color;
            }
        }
    }
}

static int cursorX = 0;
static int cursorY = 0;

struct MicroDosAPI {
  void (*print)(const char* text);
  void (*println)(const char* text);
  void (*clear)();
  void (*beep)(int freq, int ms);
  void (*delay)(int ms);
  int  (*inkey)();
  void (*color)(int colorId);
  void (*plot)(int x, int y, int colorId);
  void (*line)(int x1, int y1, int x2, int y2, int colorId);
  void (*rect)(int x, int y, int w, int h, int colorId);
  void (*circle)(int x, int y, int r, int colorId);
  int  (*peek)(int address);
  void (*poke)(int address, int value);
  int  (*getRamSize)();
  void (*pinMode)(int pin, int mode);
  void (*digitalWrite)(int pin, int val);
  int  (*digitalRead)(int pin);
  void (*inputStr)(const char* prompt, char* destBuffer, int maxLen);
  int  (*wifiUp)(const char* ssid, const char* pass);
  void (*wifiDown)();
  int  (*ollamaStream)(const char* prompt, const char* serverIp, const char* modelName, const char* sysPrompt, int streamToConsole);
  void (*getTouch)(TouchState* state);
  int termWidth;
  int termHeight;
  int  charWidth;
  int  charHeight;
  void (*drawJpeg)(const char* filename, int x, int y);
  void (*setFKeys)(const char* l1, const char* l2, const char* l3, const char* l4, const char* l5);
  void (*clearFKeys)();
  void* (*malloc)(unsigned int size);
  void  (*free)(void* ptr);
  uint32_t (*createSprite)(const char* filename);
  void  (*drawSprite)(uint32_t spriteHandle, int x, int y);
  void  (*freeSprite)(uint32_t spriteHandle);
  bool  (*initGameMatrix)();
  void  (*flushGameMatrix)();
  void  (*closeGameMatrix)();
};

static MicroDosAPI kernelAPI;

bool sdAvailable = false;

void setup() {
  Serial.begin(115200);
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) { delay(10); }
  Serial.println(F("\nStarting ESP32 MicroDOS"));

  initSD();
  Serial.println(F("- Init SD Card OK"));
  delay(100);

  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(100);
  Serial.println(F("- Init WiFi    OK"));

  tft.init();
  tft.initDMA();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  Serial.println(F("- Init TFT     OK"));

  drawKeyboard();
  drawStatusBar();

#if defined(BOARD_CYD)
  initTouchCalibration();
  randomSeed(analogRead(34));
#elif defined(BOARD_JC3248)
  randomSeed(analogRead(10));
#endif

  pinMode(0, INPUT_PULLUP);
  pinMode(AUDIO_EN_PIN, OUTPUT);
  digitalWrite(AUDIO_EN_PIN, HIGH);
  pinMode(AUDIO_DATA_PIN, OUTPUT);

  api_setup();
  Serial.println(F("- Init API     OK"));

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  kernelAPI.clear();
  clearProgram();
  printLogo();
  Serial.println(F("OK\n"));

  if (sdAvailable && SD.exists("/AUTOEXEC.BAS")) {
    terminalPrintln("EXECUTING AUTOEXEC.BAS...");
    delay(1000);
    processCommand("RUN /AUTOEXEC.BAS");
  } else {
    terminalPrint(currentPrompt);
    terminalPrint(cursor);
  }
}

void loop() {
  char pressedKey = getKeyPress(true);
  if (pressedKey != 0) {
    handleKeyPress(pressedKey);
    if (pressedKey == '\n') {
      char commandToRun[INPUT_BUF_SIZE];
      strncpy(commandToRun, inputBuffer, INPUT_BUF_SIZE);
      memset(inputBuffer, 0, INPUT_BUF_SIZE);

      processCommand(commandToRun);

      terminalPrint(currentPrompt);
      terminalPrint(cursor);
    }
  }
  delay(10);
}

#define MAX_PROGRAM_LINES 100
#define MAX_LINE_LEN 80

struct ProgramLine {
  int lineNumber;
  char code[MAX_LINE_LEN];
};
ProgramLine programMemory[MAX_PROGRAM_LINES];
int programLineCount = 0;

#define MAX_VARS 20
#define MAX_VAR_NAME_LEN 12
#define MAX_STR_VARS 20
#define MAX_STR_LEN  50

struct Variable {
  char name[MAX_VAR_NAME_LEN];
  int value;
};

Variable sysVariables[MAX_VARS];
int variableCount = 0;

struct StringVariable {
    char name[MAX_VAR_NAME_LEN];
    char value[MAX_STR_LEN];
};

static StringVariable stringRegistry[MAX_STR_VARS];
static int stringRegistryCount = 0;

#define MAX_STACK_DEPTH 20
int subroutineCallStack[MAX_STACK_DEPTH];
int stackPointer = 0;


#define MAX_FOR_NEST 4

struct ForLoopState {
  char varName;
  int targetValue;
  int stepValue;
  int lineMemoryIdx;
};

ForLoopState forStack[MAX_FOR_NEST];
int forStackPointer = 0;

static bool showThinkingLogs = false;

const char alphaLayout[KEY_ROWS][KEY_COLS] = {
  {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
  {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
  {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\t'},
  {'Z', 'X', 'C', 'V', 'B', 'N', 'M', ' ', '\r', '\n'}
};

const char symbolLayout[KEY_ROWS][KEY_COLS] = {
  {'+', '-', '*', '/', '=', '(', ')', '[', ']', '%'},
  {'"', '\'', ':', ';', '!', '?', '\\', '|', '&', '^'},
  {'.', ',', '<', '>', '_', '~', '$', '@', '#', '\t'},
  {'{', '}', ' ', ' ', ' ', ' ', ' ', ' ', '\r', '\n'}
};

static int lastTouchY = 0;
static bool touchHeld = false;

void processTerminalTouchScrolling() {
  uint16_t x = 0, y = 0;

  if (tft.getTouch(&x, &y)) {

#if defined(BOARD_CYD)
      x = TFT_WIDTH - x;
      y = TFT_HEIGHT - y;
#endif

      if (y >= 0 && y <= TFT_WIDTH) {
          if (!touchHeld) {
              touchHeld = true;
              lastTouchY = y;
          } else {
              int deltaY = y - lastTouchY;

              if (deltaY >= CHAR_HEIGHT) {
                  if (scrollOffset > 0) {
                      scrollOffset--;
                      redrawTerminal();
                  }
                  lastTouchY = y;
              }
              else if (deltaY <= -CHAR_HEIGHT) {
                  int maxPossibleScroll = activeRowIndex - TERM_ROWS + 1;
                  if (maxPossibleScroll < 0) maxPossibleScroll = 0;

                  if (scrollOffset < maxPossibleScroll) {
                      scrollOffset++;
                      redrawTerminal();
                  }
                  lastTouchY = y;
              }
          }
      }
  } else {
      touchHeld = false;
  }
}

void drawStatusBar() {
    tft.fillRect(0, STATUS_Y_START, 320, STATUS_HEIGHT, TFT_BLACK);
    tft.drawFastHLine(0, STATUS_Y_START, 320, TFT_DARKGREY);

    tft.setTextFont(2);
    tft.setTextSize(1);

    if (fKeysOverlayActive) {
        for (int i = 0; i < 5; i++) {
            int kX = i * 64;

            if (i > 0) {
                tft.drawFastVLine(kX, STATUS_Y_START + 3, STATUS_HEIGHT - 6, TFT_DARKGREY);
            }

            int strLenWidth = tft.textWidth(fKeyLabels[i]);
            int textX = kX + (64 / 2) - (strLenWidth / 2);
            int textY = STATUS_Y_START + (STATUS_HEIGHT / 2) - (CHAR_HEIGHT / 2);

            tft.setTextColor(TFT_CYAN);
            tft.setCursor(textX, textY);
            tft.print(fKeyLabels[i]);
        }
    } else {
        for (int i = 0; i < 5; i++) {
            int kX = i * 64;

            if (i > 0) {
                tft.drawFastVLine(kX, STATUS_Y_START + 3, STATUS_HEIGHT - 6, TFT_DARKGREY);
            }

            int strLenWidth = tft.textWidth(fKeyLabelsDefault[i]);
            int textX = kX + (64 / 2) - (strLenWidth / 2);
            int textY = STATUS_Y_START + (STATUS_HEIGHT / 2) - (CHAR_HEIGHT / 2);

            tft.setTextColor(TFT_CYAN);
            tft.setCursor(textX, textY);
            tft.print(fKeyLabelsDefault[i]);
	}
    }
#if defined(BOARD_JC3248)
     canvas_bridge->flush();
#endif
    tft.setTextFont(1);
    tft.setTextSize(1);
}

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  if (y >= tft.width()) return false;

  if (MATRIX_ACTIVE) {
    //TODO Decode 16-bit RGB565 to 4-bit indexed color array and push to the TFT hardware registers
    bgCanvas.pushImage(x, y, w, h, bitmap);
    bgCanvas.pushSprite(0, 0);
  } else {
    tft.pushImage(x, y, w, h, bitmap);
  }

  return true;
}

void loadJpegToScreen(const char* filename, int16_t xOut, int16_t yOut) {
  if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

  fixedFilename[0] = '\0';
  if (filename[0] != '/') {
    snprintf(fixedFilename, FILENAME_SIZE, "/%s", filename);
  } else {
    strncpy(fixedFilename, filename, FILENAME_SIZE - 1);
    fixedFilename[FILENAME_SIZE - 1] = '\0';
  }

  if (!SD.exists(fixedFilename)) {
    terminalPrintln("ERR: FILE NOT FOUND");
    return;
  }

  TJpgDec.setJpgScale(1);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  uint32_t startTime = millis();
  TJpgDec.drawSdJpg(xOut, yOut, fixedFilename);
  uint32_t duration = millis() - startTime;

#ifdef SERIAL_DEBUG
  Serial.printf("[JPEG DECODER] Rendered %s in %d ms.\n\r", fixedFilename, duration);
#endif
}

void trimCString(char* str) {
  int l = strlen(str);
  while(l > 0 && isspace((unsigned char)str[l-1])) str[--l] = '\0';
  int start = 0;
  while(str[start] && isspace((unsigned char)str[start])) start++;
  if(start > 0) memmove(str, str + start, l - start + 1);
}

void setStringVariable(const char* name, const char* val) {
    char trimmedName[MAX_VAR_NAME_LEN];
    strncpy(trimmedName, name, MAX_VAR_NAME_LEN);
    trimmedName[MAX_VAR_NAME_LEN - 1] = '\0';
    trimCString(trimmedName);

    for (int i = 0; i < stringRegistryCount; i++) {
        if (strcmp(trimmedName, stringRegistry[i].name) == 0) {
            snprintf(stringRegistry[i].value, MAX_STR_LEN, "%s", val);
            return;
        }
    }
    if (stringRegistryCount < MAX_STR_VARS) {
        snprintf(stringRegistry[stringRegistryCount].name, MAX_VAR_NAME_LEN, "%s", trimmedName);
        snprintf(stringRegistry[stringRegistryCount].value, MAX_STR_LEN, "%s", val);
        stringRegistryCount++;
    } else {
        terminalPrintln("ERR: STRING REGISTRY FULL");
    }
}

const char* getStringVariable(const char* name) {
    char trimmedName[MAX_VAR_NAME_LEN];
    strncpy(trimmedName, name, MAX_VAR_NAME_LEN);
    trimmedName[MAX_VAR_NAME_LEN-1] = '\0';
    trimCString(trimmedName);

    for (int i = 0; i < stringRegistryCount; i++) {
        if (strcmp(trimmedName, stringRegistry[i].name) == 0) {
            return stringRegistry[i].value;
        }
    }
    return "";
}

void clearConversationContext() {
    if (activeContext != NULL) {
        activeContext[0] = '\0';
    } else {
        memset(coderContext, 0, CONTEXT_BUF_SIZE);
        memset(chatContext, 0, CONTEXT_BUF_SIZE);
    }

    clearProgram();
    forStackPointer = 0;

#ifdef SERIAL_DEBUG
    if (Serial) {
        Serial.println(F("[SYSTEM: Targeted context bank cleared cleanly]"));
    }
#endif
}

void kernel_getTouchState(TouchState* state) {
  if (state == NULL) return;

  uint16_t rawX = 0;
  uint16_t rawY = 0;

  if (tft.getTouch(&rawX, &rawY)) {
#if defined(BOARD_CYD)
    int mappedX = TFT_WIDTH - rawX;
    int mappedY = TFT_HEIGHT - rawY;
#else
    int mappedX = rawX;
    int mappedY = rawY;
#endif

    if (mappedY < KEYBOARD_Y_START) {
      state->isPressed = true;
      state->x = mappedX;
      state->y = mappedY;
      return;
    }
  }

  state->isPressed = false;
  state->x = -1;
  state->y = -1;
}

char getKeyPress(bool blocking) {
  uint16_t touchX = 0, touchY = 0;
  char pressedKey = 0;
  int max_delay = 50;
  unsigned long startTime = millis();

  if (tft.getTouch(&touchX, &touchY)) {
    Serial.println(touchX);
#ifdef BOARD_CYD
    touchX = TFT_WIDTH - touchX;
    touchY = TFT_HEIGHT - touchY;
#endif

    if (touchY >= STATUS_Y_START && touchY < KEYBOARD_Y_START) {
      int fIndex = touchX / 64;

      if (fIndex >= 0 && fIndex <= 4) {
        pressedKey = fTable[fIndex];

        if (blocking) {
            while (tft.getTouch(&touchX, &touchY)) { delay(10); }
        }
        return pressedKey;
      }
    }
    else if (touchY >= KEYBOARD_Y_START && touchY < 480) {
      int localY = touchY - KEYBOARD_Y_START;
      int row = localY / KEY_HEIGHT;
      int col = touchX / KEY_WIDTH;

      if (row >= 0 && row < KEY_ROWS && col >= 0 && col < KEY_COLS) {
        pressedKey = symbolModeActive ? symbolLayout[row][col] : alphaLayout[row][col];

        if (pressedKey == '\r') {
          pressedKey = 0;
          symbolModeActive = !symbolModeActive;
          drawKeyboard();
          drawStatusBar();
          delay(250);
          return 0;
        }
        else if (pressedKey == '\t') {
          delay(200);
          return pressedKey;
        }

        if (blocking) {
            while (tft.getTouch(&touchX, &touchY)) { delay(10); }
        }
      }
    }
    else if (touchY >= 0 && touchY <= STATUS_Y_START) {
       processTerminalTouchScrolling();
    }
  } else {
      processTerminalTouchScrolling();
  }

  unsigned long execTime = millis() - startTime;
  if (execTime > 0 && execTime < max_delay) {
     delay(max_delay - execTime);
  }
  return pressedKey;
}

void playBeep(int frequency, int durationMs) {
  digitalWrite(AUDIO_EN_PIN, LOW);
  tone(AUDIO_DATA_PIN, frequency);
  delay(durationMs);
  noTone(AUDIO_DATA_PIN);
  digitalWrite(AUDIO_EN_PIN, HIGH);
}

#if defined(BOARD_CYD)
void initTouchCalibration() {
  uint16_t calData[] = { TFT_WIDTH, 3500, TFT_WIDTH, 3500, 2 };
  tft.setTouch(calData);
}
#endif

void initSD() {
#if defined(BOARD_CYD)
  sdSPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS_PIN);
  if (!SD.begin(SD_CS_PIN, sdSPI, SPI_FREQUENCY)) {
    sdAvailable = false;
  } else {
    sdAvailable = true;
  }
#elif defined(BOARD_JC3248)
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);

  if (!SD_MMC.begin("/sd", true)) {
    sdAvailable = false;
    Serial.println("SD_MMC Mount Failed!");
  } else {
    sdAvailable = true;
    Serial.println("SD_MMC Mounted Successfully!");
  }
#endif
}

void loadBuffer(const char* filename) {
  if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

  memset(fixedFilename, 0, FILENAME_SIZE);
  if (filename[0] != '/') {
    snprintf(fixedFilename, FILENAME_SIZE, "/%s", filename);
  } else {
    strncpy(fixedFilename, filename, FILENAME_SIZE);
    fixedFilename[FILENAME_SIZE - 1] = '\0';
  }

  File file = SD.open(fixedFilename);
  if (!file) { terminalPrintln("NOT FOUND"); return; }

  char line[MAX_LINE_LEN];
  int lineIdx = 0;

  while (file.available()) {
    char c = file.read();
    if (c == '\n' || lineIdx >= (int)MAX_LINE_LEN - 1) {
      line[lineIdx] = '\0';
      trimCString(line);
      terminalPrintln(line);
      lineIdx = 0;
    } else if (c != '\r') {
      line[lineIdx++] = c;
    }
  }
  file.close();
}

void loadFile(const char* filename) {
  terminalPrintln(filename);
  if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

  memset(fixedFilename, 0, FILENAME_SIZE);
  if (filename[0] != '/') {
    snprintf(fixedFilename, FILENAME_SIZE, "/%s", filename);
  } else {
    strncpy(fixedFilename, filename, FILENAME_SIZE);
    fixedFilename[FILENAME_SIZE - 1] = '\0';
  }

  File file = SD.open(fixedFilename);
  if (!file) { terminalPrintln("NOT FOUND"); return; }

  clearProgram();

  char line[MAX_LINE_LEN];
  int lineIdx = 0;

  while (file.available()) {
    char c = file.read();
    if (c == '\n' || lineIdx >= (int)MAX_LINE_LEN - 1) {
      line[lineIdx] = '\0';
      trimCString(line);

      if (strlen(line) > 0) {
        char* spaceIdx = strchr(line, ' ');
        if (spaceIdx != NULL) {
          *spaceIdx = '\0';
          int num = atoi(line);
          char* code = spaceIdx + 1;
          storeLine(num, code);
        }
      }
      lineIdx = 0;
    } else if (c != '\r') {
      line[lineIdx++] = c;
    }
  }
  file.close();
}

void saveBuffer(const char* filename) {
  if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

  memset(fixedFilename, 0, FILENAME_SIZE);
  if (filename[0] != '/') {
    snprintf(fixedFilename, FILENAME_SIZE, "/%s", filename);
  } else {
    strncpy(fixedFilename, filename, FILENAME_SIZE);
    fixedFilename[FILENAME_SIZE - 1] = '\0';
  }

  SD.remove(fixedFilename);
  File file = SD.open(fixedFilename, FILE_WRITE);

  if (!file) { terminalPrintln("WRITE FAILED"); return; }

  for (int i = 0; i < activeRowIndex-1; i++) {
    file.println(terminalBuffer[i]);
  }
  file.close();
  memset(logBuf, 0, TERM_COLS);
  snprintf(logBuf, TERM_COLS, "SAVED TO %s... ", fixedFilename);
  terminalPrint(logBuf);

  terminalPrintln("OK.");
}

void saveFile(const char* filename) {
  if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

  memset(fixedFilename, 0, FILENAME_SIZE);
  if (filename[0] != '/') {
    snprintf(fixedFilename, FILENAME_SIZE, "/%s", filename);
  } else {
    strncpy(fixedFilename, filename, FILENAME_SIZE);
    fixedFilename[FILENAME_SIZE - 1] = '\0';
  }

  memset(logBuf, 0, TERM_COLS);
  snprintf(logBuf, TERM_COLS, "SAVING TO %s... ", fixedFilename);
  terminalPrint(logBuf);
  SD.remove(fixedFilename);
  File file = SD.open(fixedFilename, FILE_WRITE);

  if (!file) { terminalPrintln("WRITE FAILED"); return; }

  for (int i = 0; i < programLineCount; i++) {
    file.print(programMemory[i].lineNumber);
    file.print(" ");
    file.println(programMemory[i].code);
  }
  file.close();
  terminalPrintln("OK.");
}

void redrawTerminal() {
  for (int r = 0; r < TERM_ROWS; r++) {
    if (MATRIX_ACTIVE) {
      bgCanvas.setCursor(0, r * CHAR_HEIGHT);
    } else {
      tft.setCursor(0, r * CHAR_HEIGHT);
    }
    int targetMemoryRow = scrollOffset + r;

    if (targetMemoryRow < TOTAL_ROWS && terminalBuffer[targetMemoryRow][0] != '\0') {
      int c = 0;
      while (terminalBuffer[targetMemoryRow][c] != '\0' && c < TERM_COLS) {
        int charColorId = colorBuffer[targetMemoryRow][c];
        if (charColorId == 0) {
	  if (MATRIX_ACTIVE) {
            bgCanvas.setTextColor(getPaletteColor(currentActivePaletteId), TFT_BLACK);
	  } else {
            tft.setTextColor(currentActivePaletteId, TFT_BLACK);
	  }
        } else {
	  if (MATRIX_ACTIVE) {
            bgCanvas.setTextColor(getPaletteColor(charColorId), TFT_BLACK);
	  } else {
            tft.setTextColor(getPaletteColor(charColorId), TFT_BLACK);
	  }
        }

        if (MATRIX_ACTIVE) {
          bgCanvas.print(terminalBuffer[targetMemoryRow][c]);
	} else {
          tft.print(terminalBuffer[targetMemoryRow][c]);
	}
        c++;
      }

      int textPixelWidth = c * CHAR_WIDTH;
      if (textPixelWidth < TFT_WIDTH) {
        if (MATRIX_ACTIVE) {
          bgCanvas.fillRect(textPixelWidth, r * CHAR_HEIGHT, TFT_WIDTH - textPixelWidth, CHAR_HEIGHT, TFT_BLACK);
	} else {
          tft.fillRect(textPixelWidth, r * CHAR_HEIGHT, TFT_WIDTH - textPixelWidth, CHAR_HEIGHT, TFT_BLACK);
	}
      }
    } else {
      if (MATRIX_ACTIVE) {
        bgCanvas.fillRect(0, r * CHAR_HEIGHT, TFT_WIDTH, CHAR_HEIGHT, TFT_BLACK);
      } else {
        tft.fillRect(0, r * CHAR_HEIGHT, TFT_WIDTH, CHAR_HEIGHT, TFT_BLACK);
      }
    }
  }
  if (MATRIX_ACTIVE) {
     bgCanvas.pushSprite(0, 0);
  } else {
#if defined(BOARD_JC3248)
     canvas_bridge->flush();
#endif
  }
}

void terminalPrintln(const char* text) {
  if (text != NULL && strlen(text) > 0) {
    terminalPrint(text);
  }

  activeRowIndex++;
  cursorX = 0;

  if (activeRowIndex >= TOTAL_ROWS) {
    for (int i = 0; i < TOTAL_ROWS - 1; i++) {
      strcpy(terminalBuffer[i], terminalBuffer[i + 1]);
      memmove(colorBuffer[i], colorBuffer[i + 1], TERM_COLS);
    }
    memset(terminalBuffer[TOTAL_ROWS-1], 0, TERM_COLS + 1);
    memset(colorBuffer[TOTAL_ROWS-1], 0, TERM_COLS);
    activeRowIndex = TOTAL_ROWS - 1;
  }

  if (activeRowIndex >= TERM_ROWS) {
      scrollOffset = activeRowIndex - TERM_ROWS + 1;
  } else {
      scrollOffset = 0;
  }

  redrawTerminal();
  cursorY = (activeRowIndex - scrollOffset) * CHAR_HEIGHT;
}

void terminalPrint(const char* text) {
  int textLen = strlen(text);
  for (int i = 0; i < textLen; i++) {
    int currentLen = strlen(terminalBuffer[activeRowIndex]);
    if (currentLen >= TERM_COLS) {
      activeRowIndex++;

      if (activeRowIndex >= TOTAL_ROWS) {
        for (int r = 0; r < TOTAL_ROWS - 1; r++) {
          strcpy(terminalBuffer[r], terminalBuffer[r + 1]);
          memmove(colorBuffer[r], colorBuffer[r + 1], TERM_COLS);
        }
        memset(terminalBuffer[TOTAL_ROWS-1], 0, TERM_COLS + 1);
        memset(colorBuffer[TOTAL_ROWS-1], 0, TERM_COLS);
        activeRowIndex = TOTAL_ROWS - 1;
      }
      currentLen = 0;
    }

    terminalBuffer[activeRowIndex][currentLen] = text[i];
    terminalBuffer[activeRowIndex][currentLen + 1] = '\0';

    colorBuffer[activeRowIndex][currentLen] = (uint8_t)currentActivePaletteId;
  }

  if (activeRowIndex >= TERM_ROWS) {
      scrollOffset = activeRowIndex - TERM_ROWS + 1;
  } else {
      scrollOffset = 0;
  }

  redrawTerminal();

  if (MATRIX_ACTIVE) {
    cursorX = bgCanvas.textWidth(terminalBuffer[activeRowIndex]);
  } else {
    cursorX = tft.textWidth(terminalBuffer[activeRowIndex]);
  }
  cursorY = (activeRowIndex - scrollOffset) * CHAR_HEIGHT;
}

void drawKeyboard() {
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setTextFont(2);
  tft.setTextSize(1);

  for (int r = 0; r < KEY_ROWS; r++) {
    for (int c = 0; c < KEY_COLS; c++) {
      int x = c * KEY_WIDTH;
      int y = KEYBOARD_Y_START + (r * KEY_HEIGHT);

      if (symbolModeActive && r == 3 && c >= 2 && c <= 7) {
        if (c == 2) {
          tft.drawRect(x, y, KEY_WIDTH * 6, KEY_HEIGHT, TFT_DARKGREY);
          tft.fillRect(x + 1, y + 1, (KEY_WIDTH * 6) - 2, KEY_HEIGHT - 2, TFT_BLUE);
          tft.drawString("  SPACE ", x + (KEY_WIDTH * 2), y + 10);
        }
        continue;
      }

      tft.drawRect(x, y, KEY_WIDTH, KEY_HEIGHT, TFT_DARKGREY);
      tft.fillRect(x + 1, y + 1, KEY_WIDTH - 2, KEY_HEIGHT - 2, TFT_BLUE);

      char key = symbolModeActive ? symbolLayout[r][c] : alphaLayout[r][c];

      if (key == '\n') {
        tft.drawString("RT", x + 6, y + 10);
      } else if (key == '\t') {
        tft.drawString("BS", x + 6, y + 10);
      } else if (key == '\r') {
        if (symbolModeActive) tft.drawString("AL", x + 6, y + 10);
        else tft.drawString("SY", x + 6, y + 10);
      } else if (key == ' ') {
        if (!symbolModeActive && c == 7) tft.drawString("SPC", x + 4, y + 10);
      } else {
	char temp[2];
	temp[0] = key;
	temp[1] = '\0';
        tft.drawString((char*)temp, x + 11, y + 10);
      }
    }
  }
#if defined(BOARD_JC3248)
     canvas_bridge->flush();
#endif
  tft.setTextColor(currentActivePaletteId, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(1);
}

void renderFullWrappedInputLine() {
    char fullLineText[INPUT_BUF_SIZE + PROMPT_SIZE + CURSOR_SIZE + 4];
    snprintf(fullLineText, sizeof(fullLineText), "%s", currentPrompt);

    strncat(fullLineText, inputBuffer, inputCursorPos);
    strcat(fullLineText, cursor);
    strcat(fullLineText, inputBuffer + inputCursorPos);

    int totalChars = strlen(fullLineText);

    int startRow = activeRowIndex;
    memset(terminalBuffer[startRow], 0, TERM_COLS + 1);
    memset(colorBuffer[startRow], 0, TERM_COLS);

    int charIdx = 0;
    int currentRow = startRow;

    while (charIdx < totalChars && currentRow < TOTAL_ROWS) {
        int chunkLen = min((int)TERM_COLS, totalChars - charIdx);
        strncpy(terminalBuffer[currentRow], fullLineText + charIdx, chunkLen);
        terminalBuffer[currentRow][chunkLen] = '\0';

        for (int i = 0; i < chunkLen; i++) {
            colorBuffer[currentRow][i] = (uint8_t)currentActivePaletteId;
        }

        charIdx += TERM_COLS;
        if (charIdx < totalChars) {
            currentRow++;
            if (currentRow < TOTAL_ROWS) {
                memset(terminalBuffer[currentRow], 0, TERM_COLS + 1);
                memset(colorBuffer[currentRow], 0, TERM_COLS);
            }
        }
    }

    if (currentRow >= TERM_ROWS) {
        scrollOffset = currentRow - TERM_ROWS + 1;
    } else {
        scrollOffset = 0;
    }
    if (cursorX < tft.textWidth(terminalBuffer[currentRow]) and cursorY > (currentRow - scrollOffset) * CHAR_HEIGHT) {
        terminalBuffer[activeRowIndex+1][0] = '\0';
    }

    redrawTerminal();

    if (MATRIX_ACTIVE) {
      cursorX = bgCanvas.textWidth(terminalBuffer[currentRow]);
    } else {
      cursorX = tft.textWidth(terminalBuffer[currentRow]);
    }
    cursorY = (currentRow - scrollOffset) * CHAR_HEIGHT;
}

void handleKeyPress(char key) {
    int inputLen = strlen(inputBuffer);

    if (key == '\n') {
      inputCursorPos = strlen(inputBuffer);
      renderFullWrappedInputLine();
      if (inputCursorPos >= (TERM_COLS - strlen(currentPrompt))) {
	 activeRowIndex++;
      }
      int len = strlen(terminalBuffer[activeRowIndex]);
      if (len > 0) {
        terminalBuffer[activeRowIndex][len - 1] = '\0';
      }
      terminalPrintln("");
      inputCursorPos = 0;
      return;
    }

    if (key == TKN_F1) {
        if (inputCursorPos > 0) {
            inputCursorPos--;
            renderFullWrappedInputLine();
        }
        return;
    }

    else if (key == TKN_F2) {
        if (inputCursorPos < inputLen) {
            inputCursorPos++;
            renderFullWrappedInputLine();
        }
        return;
    }

    else if (key == TKN_F5 || key == '\t') {
        if (inputCursorPos > 0) {
            int elementsToShift = inputLen - inputCursorPos + 1;
            memmove(&inputBuffer[inputCursorPos - 1], &inputBuffer[inputCursorPos], elementsToShift);

            inputCursorPos--;
            renderFullWrappedInputLine();
        }
        return;
    }

    else if (key >= 32 && key <= 126) {
        if (inputLen < (INPUT_BUF_SIZE - strlen(currentPrompt) - strlen(cursor))) {
            int elementsToShift = inputLen - inputCursorPos + 1;
            memmove(&inputBuffer[inputCursorPos + 1], &inputBuffer[inputCursorPos], elementsToShift);

            inputBuffer[inputCursorPos] = key;

            colorBuffer[activeRowIndex][inputCursorPos] = currentActivePaletteId;

            inputCursorPos++;

            renderFullWrappedInputLine();
        }
    }
}

struct ExecTaskParams {
  uint8_t* codeBuffer;
  uint8_t* dataBuffer;
  MicroDosAPI* api;
  volatile bool* isRunning;
  int argc;
  char** argv;
  int* exitCode;
  uint32_t entryOffset;
  uint32_t dramSize;
  TaskHandle_t hostTaskHandle;
};

struct MDBHeader {
    uint32_t dramSize;
    uint32_t iramSize;
    uint32_t entryOffset;
    uint32_t gotFileOffset;
};

void nativeBinaryTaskWorker(void* pvParameters) {
  delay(100);
  ExecTaskParams* params = (ExecTaskParams*)pvParameters;

  uint32_t trueEntryAddress = (uint32_t)params->codeBuffer + (params->entryOffset - sizeof(MDBHeader));

  typedef int (*AppEntryPoint)(int argc, char** argv, MicroDosAPI* api);
  AppEntryPoint run_app = (AppEntryPoint)trueEntryAddress;

  int runtimeExitStatus = run_app(params->argc, params->argv, params->api);

  if (params->exitCode)   *(params->exitCode) = runtimeExitStatus;

  volatile bool* syncFlag = params->isRunning;

  if (syncFlag) {
      *syncFlag = false;
  }

  vTaskDelete(NULL);
}

void printLogo() {
  kernelAPI.clear();
  uint32_t freeRam = ESP.getFreeHeap();
  memset(logBuf, 0, TERM_COLS);
  snprintf(logBuf, TERM_COLS, "ESP32 MICRO-DOS %u BYTES FREE", freeRam);
  terminalPrintln(logBuf);
  terminalPrintln("READY.");
}

void clearProgram() {
  for (int i = 0; i < MAX_PROGRAM_LINES; i++) {
    programMemory[i].lineNumber = 0;
    programMemory[i].code[0] = '\0';
  }
  for (int i = 0; i < SYSTEM_RAM_SIZE; i++) {
    systemRAM[i] = 0;
  }
  stringRegistryCount = 0;
  programLineCount = 0;
  variableCount = 0;
  stackPointer = 0;
  forStackPointer = 0;
}

bool matchFuncBounds(const char* str, const char* prefix, int prefixLen, int &innerLen) {
  int totalLen = strlen(str);
  if (totalLen >= prefixLen + 1 && strncmp(str, prefix, prefixLen) == 0 && str[totalLen - 1] == ')') {
    innerLen = totalLen - prefixLen - 1;
    return true;
  }
  return false;
}

int evaluateExpression(const char* rawExpr) {
  char expr[MAX_LINE_LEN];
  strncpy(expr, rawExpr, MAX_LINE_LEN);
  expr[MAX_LINE_LEN - 1] = '\0';
  trimCString(expr);

  int len = strlen(expr);
  if (len == 0) return 0;

  if (strcasecmp(expr, "TRUE") == 0)  return 1;
  if (strcasecmp(expr, "FALSE") == 0) return 0;

  if (strcmp(expr, "RND") == 0) {
    return random(0, 100);
  }

  int innerLen = 0;
  if (matchFuncBounds(expr, "RND(", 4, innerLen)) {
    expr[4 + innerLen] = '\0';
    int limit = evaluateExpression(expr + 4);
    return (limit > 0) ? random(0, limit) : 0;
  }

  if (matchFuncBounds(expr, "INT(", 4, innerLen)) {
    expr[4 + innerLen] = '\0';
    return evaluateExpression(expr + 4);
  }

  if (matchFuncBounds(expr, "SQR(", 4, innerLen)) {
    expr[4 + innerLen] = '\0';
    int val = evaluateExpression(expr + 4);
    return (val > 0) ? (int)sqrt(val) : 0;
  }

  if (matchFuncBounds(expr, "SIN(", 4, innerLen)) {
    expr[4 + innerLen] = '\0';
    return (int)(sin(evaluateExpression(expr + 4) * DEG_TO_RAD) * 100.0);
  }

  if (matchFuncBounds(expr, "COS(", 4, innerLen)) {
    expr[4 + innerLen] = '\0';
    return (int)(cos(evaluateExpression(expr + 4) * DEG_TO_RAD) * 100.0);
  }

  if (matchFuncBounds(expr, "LN(", 3, innerLen)) {
    expr[3 + innerLen] = '\0';
    int val = evaluateExpression(expr + 3);
    return (val > 0) ? (int)(log(val) * 100.0) : 0;
  }

  if (matchFuncBounds(expr, "PEEK(", 5, innerLen)) {
    expr[5 + innerLen] = '\0';
    int address = evaluateExpression(expr + 5);
    if (address >= 0 && address < SYSTEM_RAM_SIZE) return systemRAM[address];
    terminalPrintln("ERR: RAM OUT OF BOUNDS"); return 0;
  }

  if (matchFuncBounds(expr, "INREAD(", 7, innerLen)) {
    expr[7 + innerLen] = '\0';
    int pin = evaluateExpression(expr + 7);
    if (pin != 5 && pin != 12 && pin != 13 && pin != 14 && pin != 15 && pin != 18 && pin != 19 && pin != 23) {
      if (pin == 34 || pin == 35 || pin == 36 || pin == 39) {
        return analogRead(pin);
      } else {
        return digitalRead(pin);
      }
    }
    terminalPrintln("ERR: RESERVED CORE BUS PIN");
    return 0;
  }

  if (matchFuncBounds(expr, "VAL(", 4, innerLen)) {
    expr[4 + innerLen] = '\0';
    char* inner = expr + 4;
    trimCString(inner);
    int ilen = strlen(inner);
    if (ilen > 0 && inner[ilen - 1] == '$') {
      return atoi(getStringVariable(inner));
    }
    if (ilen >= 2 && inner[0] == '"' && inner[ilen - 1] == '"') {
      inner[ilen - 1] = '\0';
      return atoi(inner + 1);
    }
  }

  int opIdx = -1; char foundOp = ' '; int parenDepth = 0;
  for (int i = len - 1; i >= 0; i--) {
    char c = expr[i];
    if (c == ')') parenDepth++;
    else if (c == '(') parenDepth--;
    else if (parenDepth == 0 && (c == '+' || c == '-')) { opIdx = i; foundOp = c; break; }
  }

  if (opIdx == -1) {
    parenDepth = 0;
    for (int i = len - 1; i >= 0; i--) {
      char c = expr[i];
      if (c == ')') parenDepth++;
      else if (c == '(') parenDepth--;
      else if (parenDepth == 0 && (c == '*' || c == '/' || c == '%')) {
        opIdx = i;
        foundOp = c;
        break;
      }
    }
  }

  if (opIdx != -1) {
    expr[opIdx] = '\0';
    int leftVal = evaluateExpression(expr);
    int rightVal = evaluateExpression(expr + opIdx + 1);
    switch (foundOp) {
      case '+': return leftVal + rightVal;
      case '-': return leftVal - rightVal;
      case '*': return leftVal * rightVal;
      case '/': return (rightVal != 0) ? leftVal / rightVal : 0;
      case '%': return (rightVal != 0) ? leftVal % rightVal : 0;
    }
  }

  if (len > 0 && isAlpha((unsigned char)expr[0])) {
    bool validVar = true;
    for (int i = 1; i < len; i++) {
      if (!isAlphaNumeric((unsigned char)expr[i])) { validVar = false; break; }
    }
    if (validVar) return getVariable(expr);
  }
  return atoi(expr);
}

int getVariable(const char* name) {
  char upperName[MAX_VAR_NAME_LEN];
  strncpy(upperName, name, MAX_VAR_NAME_LEN);
  upperName[MAX_VAR_NAME_LEN - 1] = '\0';
  for (int i = 0; upperName[i]; i++) upperName[i] = toupper((unsigned char)upperName[i]);

  for (int i = 0; i < variableCount; i++) {
    if (strcmp(sysVariables[i].name, upperName) == 0) return sysVariables[i].value;
  }
  return 0;
}

void setVariable(const char* name, int val) {
  char upperName[MAX_VAR_NAME_LEN];
  strncpy(upperName, name, MAX_VAR_NAME_LEN);
  upperName[MAX_VAR_NAME_LEN - 1] = '\0';
  for (int i = 0; upperName[i]; i++) upperName[i] = toupper((unsigned char)upperName[i]);

  for (int i = 0; i < variableCount; i++) {
    if (strcmp(sysVariables[i].name, upperName) == 0) { sysVariables[i].value = val; return; }
  }
  if (variableCount < MAX_VARS) {
    strncpy(sysVariables[variableCount].name, upperName, sizeof(sysVariables[variableCount].name));
    sysVariables[variableCount].value = val;
    variableCount++;
  }
}

void storeLine(int num, const char* codeLine) {
  char cleanLine[MAX_LINE_LEN];
  strncpy(cleanLine, codeLine, MAX_LINE_LEN);
  cleanLine[MAX_LINE_LEN - 1] = '\0';
  trimCString(cleanLine);

  if (strlen(cleanLine) == 0) {
    for (int i = 0; i < programLineCount; i++) {
      if (programMemory[i].lineNumber == num) {
        for (int j = i; j < programLineCount - 1; j++) programMemory[j] = programMemory[j + 1];
        programLineCount--;
        return;
      }
    }
    return;
  }
  for (int i = 0; i < programLineCount; i++) {
    if (programMemory[i].lineNumber == num) {
      strncpy(programMemory[i].code, cleanLine, MAX_LINE_LEN);
      programMemory[i].code[MAX_LINE_LEN - 1] = '\0';
      return;
    }
  }
  if (programLineCount < MAX_PROGRAM_LINES) {
    int insertPos = programLineCount;
    for (int i = 0; i < programLineCount; i++) {
      if (programMemory[i].lineNumber > num) { insertPos = i; break; }
    }
    for (int i = programLineCount; i > insertPos; i--) programMemory[i] = programMemory[i - 1];
    programMemory[insertPos].lineNumber = num;
    strncpy(programMemory[insertPos].code, cleanLine, MAX_LINE_LEN);
    programMemory[insertPos].code[MAX_LINE_LEN - 1] = '\0';
    programLineCount++;
  } else {
    terminalPrintln("ERR: MEM FULL");
  }
}

void listProgramRange(int start, int end) {
  if (programLineCount == 0) { terminalPrintln("NO PROG."); return; }
  int printedCount = 0;
  char listBuf[MAX_LINE_LEN];
  for (int i = 0; i < programLineCount; i++) {
    int currentNum = programMemory[i].lineNumber;
    if (currentNum >= start && currentNum <= end) {
      snprintf(listBuf, MAX_LINE_LEN, "%d %s", currentNum, programMemory[i].code);
      terminalPrintln(listBuf);
      printedCount++;
    }
  }
  if (printedCount == 0) terminalPrintln("LINE NOT FOUND.");
}

void listProgram() {
  if (programLineCount == 0) { terminalPrintln("NO PROG."); return; }
  char listBuf[MAX_LINE_LEN];
  for (int i = 0; i < programLineCount; i++) {
    snprintf(listBuf, MAX_LINE_LEN, "%d %s", programMemory[i].lineNumber, programMemory[i].code);
    terminalPrintln(listBuf);
  }
}

bool runMultiStatementLine(const char* fullLine, int &nextIdx) {
  char cleanLine[MAX_LINE_LEN + PROMPT_SIZE + CURSOR_SIZE];
  strncpy(cleanLine, fullLine, (MAX_LINE_LEN + PROMPT_SIZE + CURSOR_SIZE));
  cleanLine[(MAX_LINE_LEN + PROMPT_SIZE + CURSOR_SIZE) - 1] = '\0';
  trimCString(cleanLine);

  bool res = true;
  int totalLen = (MAX_LINE_LEN + PROMPT_SIZE + CURSOR_SIZE);
  if (totalLen == 0) return res;

  bool inQuotes = false;
  int startIdx = 0;

  for (int i = 0; i < totalLen; i++) {
    char c = cleanLine[i];
    if (c == '"') inQuotes = !inQuotes;

    if ((c == ':' && !inQuotes) || i == totalLen - 1) {
      int endIdx = (c == ':' && !inQuotes) ? i : i + 1;

      char statement[MAX_LINE_LEN];
      int chunkLen = endIdx - startIdx;
      if (chunkLen >= (int)MAX_LINE_LEN) chunkLen = MAX_LINE_LEN - 1;

      strncpy(statement, cleanLine + startIdx, chunkLen);
      statement[chunkLen] = '\0';
      trimCString(statement);

      if (strlen(statement) > 0) {
        int dummyIdx = nextIdx;
        res = runSingleLine(statement, dummyIdx);
        if (!res) return res;

        if (dummyIdx != nextIdx) {
          nextIdx = dummyIdx;
          return res;
        }
      }
      startIdx = i + 1;
    }
  }
  return res;
}
void executeProgram() {
  int currentIdx = 0;
  variableCount = 0;
  memset(logBuf, 0, TERM_COLS);

  while (currentIdx < programLineCount) {
    if (digitalRead(0) == LOW) {
      snprintf(logBuf, TERM_COLS, "\nBREAK AT LINE %d", programMemory[currentIdx].lineNumber);
      terminalPrintln(logBuf);
      while (digitalRead(0) == LOW) { delay(10); }
      break;
    }

    int trackingIdx = currentIdx;
    if (!runMultiStatementLine(programMemory[currentIdx].code, trackingIdx)) {
      snprintf(logBuf, TERM_COLS, "HALT AT %d", programMemory[currentIdx].lineNumber);
      terminalPrintln(logBuf);
      break;
    }

    if (trackingIdx == currentIdx) {
      currentIdx++;
    } else {
      currentIdx = trackingIdx;
    }
    delay(1);
  }
}

void stripQuotes(char* str) {
  int i = 0, j = 0;
  while (str[i]) {
    if (str[i] != '"') {
      str[j++] = str[i];
    }
    i++;
  }
  str[j] = '\0';
}

void processCommand(const char* rawCmd) {
 char cmd[MAX_LINE_LEN];
 strncpy(cmd, rawCmd, MAX_LINE_LEN);
 cmd[MAX_LINE_LEN - 1] = '\0';
 trimCString(cmd);

 if (strlen(cmd) == 0) return;

 bool evaluationPending = true;
 while (evaluationPending) {

  char firstToken[16];
  char* firstSpace = strchr(cmd, ' ');

  if (firstSpace != NULL) {
    int tokenLen = firstSpace - cmd;
    if (tokenLen >= (int)16) tokenLen = 16 - 1;
    strncpy(firstToken, cmd, tokenLen);
    firstToken[tokenLen] = '\0';
  } else {
    strncpy(firstToken, cmd, 16);
    firstToken[16 - 1] = '\0';
  }

  for (int i = 0; firstToken[i]; i++) {
    firstToken[i] = toupper((unsigned char)firstToken[i]);
  }

  bool isLineNumber = true;
  for (int i = 0; firstToken[i]; i++) {
    if (!isdigit((unsigned char)firstToken[i])) { isLineNumber = false; break; }
  }

  if (isLineNumber) {
    int lineNum = atoi(firstToken);
    const char* codeBody = (firstSpace != NULL) ? (firstSpace + 1) : "";
    storeLine(lineNum, codeBody);
    return;
  }

  if (strcmp(firstToken, "DIR") == 0 || strcmp(firstToken, "CAT") == 0) {
    if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

    File root = SD.open("/");
    if (!root) { terminalPrintln("ERR: OPEN FAILED"); return; }
    if (!root.isDirectory()) { root.close(); terminalPrintln("NOT A DIR"); return; }

    File file = root.openNextFile();
    int count = 0;

    char localDirBuf[TERM_COLS];

    while (file) {
      if (!file.isDirectory()) {
        snprintf(localDirBuf, TERM_COLS, " %s (%d B)", file.name(), (int)file.size());
        terminalPrintln(localDirBuf);
        count++;
      }
      file.close();
      file = root.openNextFile();
    }
    root.close();

    snprintf(localDirBuf, TERM_COLS, "%d FILES FOUND.", count);
    terminalPrintln(localDirBuf);
    return;
  }
  else if (strcmp(firstToken, "DELETE") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);

      memset(fixedFilename, 0, FILENAME_SIZE);
      if (baseFilename[0] != '/') {
        snprintf(fixedFilename, FILENAME_SIZE, "/%s", baseFilename);
      } else {
        strncpy(fixedFilename, baseFilename, FILENAME_SIZE);
        fixedFilename[FILENAME_SIZE - 1] = '\0';
      }

      if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

      if (SD.exists(fixedFilename)) {
        if (SD.remove(fixedFilename)) {
          terminalPrintln("FILE ERASED CLEANLY.");
        } else {
          terminalPrintln("ERR: WRITE PROTECTED OR LOCKED");
        }
      } else {
        terminalPrintln("ERR: FILE NOT FOUND");
      }
    } else {
      terminalPrintln("ERR: SPECIFY FILENAME");
    }
    return;
  }
  else if (strcmp(firstToken, "LIST") == 0) {
    int startLine = 0;
    int endLine = 999999;
    if (firstSpace != NULL) {
      char args[32];
      strncpy(args, firstSpace + 1, 32);
      args[32 - 1] = '\0';
      trimCString(args);

      char* commaIdx = strchr(args, ',');
      if (commaIdx != NULL) {
        *commaIdx = '\0';
        startLine = atoi(args);
        endLine = atoi(commaIdx + 1);
      } else {
        startLine = atoi(args);
        endLine = startLine;
      }
    }
    listProgramRange(startLine, endLine);
    return;
  }
  else if (strcmp(firstToken, "SAVE$") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);
      saveBuffer(baseFilename);
    } else { terminalPrintln("ERR: SPECIFY FILE"); }
    return;
  }
  else if (strcmp(firstToken, "LOAD$") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);
      loadBuffer(baseFilename);
    } else { terminalPrintln("ERR: SPECIFY FILE"); }
    return;
  }
  else if (strcmp(firstToken, "LOAD") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);
      loadFile(baseFilename);
    } else { terminalPrintln("ERR: SPECIFY FILE"); }
    return;
  }
  else if (strcmp(firstToken, "SAVE") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);
      saveFile(baseFilename);
    } else { terminalPrintln("ERR: SPECIFY FILE"); }
    return;
  }
  else if (strcmp(firstToken, "RUN") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);
      loadFile(baseFilename);
    }
    executeProgram();
    return;
  }
  else if (strcmp(firstToken, "DUMP") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);

      memset(fixedFilename, 0, FILENAME_SIZE);
      if (baseFilename[0] != '/') {
        snprintf(fixedFilename, FILENAME_SIZE, "/%s", baseFilename);
      } else {
        strncpy(fixedFilename, baseFilename, FILENAME_SIZE);
        fixedFilename[FILENAME_SIZE - 1] = '\0';
      }

      if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }
      File file = SD.open(fixedFilename, FILE_READ);
      if (!file) { terminalPrintln("NOT FOUND"); return; }

      uint32_t address = 0;
      uint8_t lineBuffer[16];

      while (file.available()) {
        if (address % 256 == 0) {
          delay(2);
          if (digitalRead(0) == LOW) { terminalPrintln("\nDUMP ABORTED."); break; }
        }

        int bytesRead = file.read(lineBuffer, 16);
        if (bytesRead <= 0) break;

        char addrStr[12];
        snprintf(addrStr, 12, "%07x ", (unsigned int)address);
        terminalPrint(addrStr);

        for (int i = 0; i < 16; i += 2) {
          char hexStr[8];
          if (i < bytesRead) {
            uint8_t lowByte = lineBuffer[i];
            uint8_t highByte = (i + 1 < bytesRead) ? lineBuffer[i + 1] : 0x00;
            if (i + 1 < bytesRead) {
              snprintf(hexStr, 8, "%02x%02x ", lowByte, highByte);
            } else {
              snprintf(hexStr, 8, "%02x   ", lowByte);
            }
            terminalPrint(hexStr);
          }
        }
        terminalPrintln("");
        address += bytesRead;
      }
      file.close();
    } else { terminalPrintln("ERR: SPECIFY FILENAME"); }
    return;
  }
  else if (strcmp(firstToken, "DUMPS") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);

      memset(fixedFilename, 0, FILENAME_SIZE);
      if (baseFilename[0] != '/') {
        snprintf(fixedFilename, FILENAME_SIZE, "/%s", baseFilename);
      } else {
        strncpy(fixedFilename, baseFilename, FILENAME_SIZE);
        fixedFilename[FILENAME_SIZE - 1] = '\0';
      }

      if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }
      File file = SD.open(fixedFilename, FILE_READ);
      if (!file) { terminalPrintln("NOT FOUND"); return; }

      uint32_t address = 0;
      uint8_t stringBuffer[32];

      while (file.available()) {
        if (address % 256 == 0) {
          delay(2);
          if (digitalRead(0) == LOW) { terminalPrintln("\nDUMP ABORTED."); break; }
        }

        int bytesRead = file.read(stringBuffer, 32);
        if (bytesRead <= 0) break;

        char addrStr[14];
        snprintf(addrStr, 14, "%07x | ", (unsigned int)address);
        terminalPrint(addrStr);

        char asciiBuf[2] = {0, 0};
        for (int i = 0; i < bytesRead; i++) {
          char c = stringBuffer[i];
          if (c >= 32 && c <= 126) {
            asciiBuf[0] = c;
            terminalPrint(asciiBuf);
          } else {
            terminalPrint(".");
          }
        }
        terminalPrintln("");
        address += bytesRead;
      }
      file.close();
    }
    return;
  }
  else if (strcmp(firstToken, "IMVIEW") == 0) {
    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);

      loadJpegToScreen(baseFilename, 0, 0);

      getKeyPress(true);

      redrawTerminal();
    } else {
      terminalPrintln("ERR: SPECIFY IMAGE FILENAME");
    }
    return;
  }
  else if (strcmp(firstToken, "EDIT") == 0) {
    strncpy(previousPrompt, currentPrompt, PROMPT_SIZE - 1);
    previousPrompt[PROMPT_SIZE - 1] = '\0';

    strncpy(currentPrompt, "ED> ", PROMPT_SIZE - 1);
    currentPrompt[PROMPT_SIZE - 1] = '\0';

    if (firstSpace != NULL) {
      memset(baseFilename, 0, FILENAME_SIZE);
      strncpy(baseFilename, firstSpace + 1, FILENAME_SIZE - 1);
      baseFilename[FILENAME_SIZE - 1] = '\0';
      stripQuotes(baseFilename);
      trimCString(baseFilename);

      memset(fixedFilename, 0, FILENAME_SIZE);
      if (baseFilename[0] != '/') {
        snprintf(fixedFilename, FILENAME_SIZE, "/%s", baseFilename);
      } else {
        strncpy(fixedFilename, baseFilename, FILENAME_SIZE - 1);
        fixedFilename[FILENAME_SIZE - 1] = '\0';
      }

      terminalPrint("EDITING: ");
      terminalPrintln(fixedFilename);
      terminalPrintln("TYPE H FOR HEP, L TO LIST, A TO APPEND,");
      terminalPrintln("E TO EDIT, S TO SAVE & EXIT, Q TO QUIT.");

      #define MAX_EDIT_LINES 250
      String lines[MAX_EDIT_LINES];
      int fileLineCount = 0;

      if (sdAvailable && SD.exists(fixedFilename)) {
        File f = SD.open(fixedFilename, FILE_READ);
        if (f) {
          while (f.available() && fileLineCount < MAX_EDIT_LINES) {
            String l = f.readStringUntil('\n');
            l.trim();
            lines[fileLineCount++] = l;
          }
          f.close();

          snprintf(logBuf, TERM_COLS, "LOADED %d LINES.", fileLineCount);
          terminalPrintln(logBuf);
        }
      }

      bool editing = true;
      while (editing) {
        if (activeRowIndex >= TERM_ROWS) {
            scrollOffset = activeRowIndex - TERM_ROWS + 1;
        } else {
            scrollOffset = 0;
        }

        char initialPrompt[PROMPT_SIZE + 4];
        snprintf(initialPrompt, PROMPT_SIZE, "%s%s", currentPrompt, cursor);
        terminalPrint(initialPrompt);

        memset(inputBuffer, 0, INPUT_BUF_SIZE);
        String userEntry = "";
        bool readingInput = true;

        while (readingInput) {
          char pressedKey = getKeyPress(true);
          if (pressedKey > 0) {
            handleKeyPress(pressedKey);
            if (pressedKey == '\n') {
              userEntry = String(inputBuffer);
              memset(inputBuffer, 0, INPUT_BUF_SIZE);
              readingInput = false;
            }
          }
          delay(10);
        }

        userEntry.trim();
        if (userEntry.equalsIgnoreCase("L") || userEntry.substring(0, 2).equalsIgnoreCase("L ")) {
          if (fileLineCount == 0) {
            terminalPrintln("[EMPTY]");
          } else {
            int startLine = 0;
            int endLine = fileLineCount;
            if (userEntry.substring(0, 2).equalsIgnoreCase("L ")) {
              int spIdx = userEntry.indexOf(' ');
              if (spIdx != -1) {
                String args = userEntry.substring(spIdx + 1);
                args.trim();
                int commaIdx = args.indexOf(',');
                if (commaIdx != -1) {
                  startLine = args.substring(0, commaIdx).toInt();
                  if (startLine > 0) startLine -= 1;
                  endLine = args.substring(commaIdx + 1).toInt();
                } else {
                  startLine = args.toInt();
                  if (startLine > 0) startLine -= 1;
                  endLine = startLine + 1;
                }
              }
            }

            if (startLine < 0) startLine = 0;
            if (endLine > fileLineCount) endLine = fileLineCount;

            for (int i = startLine; i < endLine; i++) {
              terminalPrintln((String(i + 1) + ": " + lines[i]).c_str());
            }
          }
	  continue;
        }
        else if (userEntry.equalsIgnoreCase("A")) {
          terminalPrintln("APPENDING MODE. ENTER BLANK LINE TO EXIT.");

          char tempPrompt[PROMPT_SIZE] = "";
          strncpy(tempPrompt, (String(fileLineCount + 1) + "+ ").c_str(), PROMPT_SIZE);
          strncpy(currentPrompt, tempPrompt, PROMPT_SIZE - 1);
          currentPrompt[PROMPT_SIZE - 1] = '\0';
	  terminalPrint(currentPrompt);
	  terminalPrint(cursor);

          while (fileLineCount < MAX_EDIT_LINES) {
            if (activeRowIndex >= TERM_ROWS) scrollOffset = activeRowIndex - TERM_ROWS + 1;
            String appendLine = "";
            memset(inputBuffer, 0, INPUT_BUF_SIZE);
            bool readingLine = true;
            while (readingLine) {
              char pressedKey = getKeyPress(true);
              if (pressedKey > 0) {
                handleKeyPress(pressedKey);
                if (pressedKey == '\n') {
                  readingLine = false;
                  appendLine = String(inputBuffer);
                  memset(inputBuffer, 0, INPUT_BUF_SIZE);
                }
              }
              delay(10);
            }

            appendLine.trim();
            if (appendLine.length() == 0) {
                strncpy(currentPrompt, "ED> ", PROMPT_SIZE - 1);
                currentPrompt[PROMPT_SIZE - 1] = '\0';
	        break;
	    }
            lines[fileLineCount++] = appendLine;
            strncpy(tempPrompt, (String(fileLineCount + 1) + "+ ").c_str(), PROMPT_SIZE);
            strncpy(currentPrompt, tempPrompt, PROMPT_SIZE - 1);
            currentPrompt[PROMPT_SIZE - 1] = '\0';
	    terminalPrint(currentPrompt);
	    terminalPrint(cursor);
          }
	  continue;
        }
        else if (userEntry.equalsIgnoreCase("S")) {
          if (sdAvailable) {
            SD.remove(fixedFilename);
            File f = SD.open(fixedFilename, FILE_WRITE);
            if (f) {
              for (int i = 0; i < fileLineCount; i++) {
                f.println(lines[i]);
              }
              f.close();
              terminalPrintln("FILE SAVED.");
            } else {
              terminalPrintln("WRITE FAILED.");
            }
          }
          editing = false;
	  continue;
        }
        if (userEntry.equalsIgnoreCase("E") || userEntry.substring(0, 2).equalsIgnoreCase("E ")) {
          if (userEntry.equalsIgnoreCase("E")) {
            terminalPrintln("USAGE: E line");
	  } else {
	    int editLine = 0;
            int spIdx = userEntry.indexOf(' ');
            if (spIdx != -1) {
              String args = userEntry.substring(spIdx + 1);
              args.trim();
              editLine = args.toInt();
            }
            if (editLine < 1 || editLine > fileLineCount) {
              terminalPrintln("ERR: OUT OF RANGE");
	    } else {
              terminalPrintln("EDIT MODE. ENTER TO SAVE & EXIT.");
              char tempPrompt[PROMPT_SIZE] = "";
              strncpy(tempPrompt, (String(editLine) + "* ").c_str(), PROMPT_SIZE);
              strncpy(currentPrompt, tempPrompt, PROMPT_SIZE);
              currentPrompt[PROMPT_SIZE - 1] = '\0';
	      terminalPrint(currentPrompt);

              if (activeRowIndex >= TERM_ROWS) scrollOffset = activeRowIndex - TERM_ROWS + 1;
              memset(inputBuffer, 0, INPUT_BUF_SIZE);
              memcpy(inputBuffer, lines[editLine-1].c_str(), INPUT_BUF_SIZE);
  	      terminalPrint(inputBuffer);
	      terminalPrint(cursor);
              inputCursorPos = strlen(inputBuffer);
	      if (inputCursorPos >= (TERM_COLS - strlen(currentPrompt))) {
		 activeRowIndex--;
	      }
              bool editingLine = true;
              while (editingLine) {
                char pressedKey = getKeyPress(true);
                if (pressedKey > 0) {
                  handleKeyPress(pressedKey);
                  if (pressedKey == '\n') {
                    lines[editLine-1] =  String(inputBuffer);
                    memset(inputBuffer, 0, INPUT_BUF_SIZE);
                    strncpy(currentPrompt, "ED> ", PROMPT_SIZE);
                    currentPrompt[PROMPT_SIZE - 1] = '\0';
                    editingLine = false;
                  }
                }
                delay(10);
              }
	    }
          }
	  continue;
        }
        else if (userEntry.equalsIgnoreCase("H")) {
          terminalPrintln("TYPE H FOR HEP, L TO LIST, A TO APPEND,");
          terminalPrintln("E TO EDIT, S TO SAVE & EXIT, Q TO QUIT.");
	  continue;
	}
        else if (userEntry.equalsIgnoreCase("Q")) {
          terminalPrintln("CHANGES DISCARDED.");
          editing = false;
	  continue;
        }
        else  {
          terminalPrintln("ERR: UNKNOWN COMMAND.");
	}
      }
    } else {
      terminalPrintln("ERR: SPECIFY FILENAME");
    }

    strncpy(currentPrompt, previousPrompt, PROMPT_SIZE);
    currentPrompt[PROMPT_SIZE - 1] = '\0';
    return;
  }
  else if (strcmp(firstToken, "EXEC") == 0) {
    if (firstSpace != NULL) {
      char restOfCmd[MAX_LINE_LEN];
      strncpy(restOfCmd, firstSpace + 1, MAX_LINE_LEN);
      restOfCmd[MAX_LINE_LEN - 1] = '\0';
      trimCString(restOfCmd);

      int argc = 0;
      const int MAX_ARGS = 8;
      char argTokens[MAX_ARGS][FILENAME_SIZE];
      for(int i = 0; i < MAX_ARGS; i++) argTokens[i][0] = '\0';

      char* nextSpace = strchr(restOfCmd, ' ');
      memset(baseFilename, 0, FILENAME_SIZE);
      if (nextSpace == NULL) {
        strlcpy(baseFilename, restOfCmd, FILENAME_SIZE);
      } else {
        int flen = nextSpace - restOfCmd;
        if (flen >= (int)FILENAME_SIZE) flen = FILENAME_SIZE - 1;
        strncpy(baseFilename, restOfCmd, flen);
        baseFilename[flen] = '\0';
      }
      stripQuotes(baseFilename);
      trimCString(baseFilename);

      memset(fixedFilename, 0, FILENAME_SIZE);
      if (baseFilename[0] != '/') {
        snprintf(fixedFilename, FILENAME_SIZE, "/%s", baseFilename);
      } else {
        strncpy(fixedFilename, baseFilename, FILENAME_SIZE);
        fixedFilename[FILENAME_SIZE - 1] = '\0';
      }

      strlcpy(argTokens[argc++], fixedFilename, FILENAME_SIZE);

      if (nextSpace != NULL) {
        char remainingArgs[MAX_LINE_LEN];
        strncpy(remainingArgs, nextSpace + 1, MAX_LINE_LEN);
        remainingArgs[MAX_LINE_LEN - 1] = '\0';
        trimCString(remainingArgs);

        while (strlen(remainingArgs) > 0 && argc < MAX_ARGS) {
          char* sp = strchr(remainingArgs, ' ');
          if (sp == NULL) {
            strlcpy(argTokens[argc++], remainingArgs, FILENAME_SIZE);
            break;
          } else {
            *sp = '\0';
            strlcpy(argTokens[argc++], remainingArgs, FILENAME_SIZE);
            char tmp[MAX_LINE_LEN];
            strncpy(tmp, sp + 1, MAX_LINE_LEN);
            tmp[MAX_LINE_LEN - 1] = '\0';
            trimCString(tmp);
            strcpy(remainingArgs, tmp);
          }
        }
      }

      if (!sdAvailable) { terminalPrintln("ERR: NO DISK"); return; }

      File file = SD.open(fixedFilename, FILE_READ);
      if (!file) { terminalPrintln("NOT FOUND"); return; }

      MDBHeader header;
      if (file.read((uint8_t*)&header, sizeof(MDBHeader)) != sizeof(MDBHeader)) {
          terminalPrintln("ERR: INVALID HEADER READ");
          file.close();
          return;
      }

      size_t argvArraySize = (argc + 1) * sizeof(char*);
      size_t argsStringsSize = 0;
      for (int i = 0; i < argc; i++) argsStringsSize += (strlen(argTokens[i]) + 1);
      size_t alignedArgsStringsSize = (argsStringsSize + 3) & ~3;

      size_t totalDramAllocation = header.dramSize + argvArraySize + alignedArgsStringsSize;
      uint8_t* localDramBuffer = (uint8_t*)malloc(totalDramAllocation);
      uint8_t* localIramBuffer = (uint8_t*)heap_caps_malloc(header.iramSize, MALLOC_CAP_32BIT | MALLOC_CAP_EXEC);
      uint8_t* iramStagingArea = (uint8_t*)malloc(header.iramSize);

      if (localDramBuffer == NULL || localIramBuffer == NULL || iramStagingArea == NULL) {
          terminalPrintln("ERR: OUT OF RAM");
	  Serial.println("ERR: OUT OF RAM");
          if (localDramBuffer) free(localDramBuffer);
          if (localIramBuffer) heap_caps_free(localIramBuffer);
          if (iramStagingArea) free(iramStagingArea);
          file.close();
          return;
      }

      memset(localDramBuffer, 0, totalDramAllocation);

      // ============================================================================
      // THE KERNEL SEGMENT LOADER
      // ============================================================================
      file.seek(sizeof(MDBHeader));
      file.read(iramStagingArea, header.iramSize);

      file.seek(sizeof(MDBHeader) + header.iramSize);
      file.read(localDramBuffer, header.dramSize);

      file.close();

#ifdef SERIAL_DEBUG
      // ============================================================================
      // DIAGNOSTIC BLOCK: HEADER & ALLOCATION BLUEPRINT
      // ============================================================================
      Serial.printf("\n\r--- [MDB DEBUG: %s] ---\n\r", fixedFilename);
      Serial.printf("Header - dramSize:    %d (0x%04X)\n\r", header.dramSize, header.dramSize);
      Serial.printf("Header - iramSize:    %d (0x%04X)\n\r", header.iramSize, header.iramSize);
      Serial.printf("Header - entryOffset: %d (0x%04X)\n\r", header.entryOffset, header.entryOffset);
      Serial.printf("Header - gotOffset:   %d (0x%04X)\n\r", header.gotFileOffset, header.gotFileOffset);
      Serial.printf("Runtime - localDramBuffer Alloc Base: 0x%08X\n\r", (uint32_t)localDramBuffer);
      Serial.printf("Runtime - localIramBuffer Alloc Base: 0x%08X\n\r", (uint32_t)localIramBuffer);

      Serial.printf("\nScanning Literal Table Entries across full IRAM...\n\r");
#endif
      // ============================================================================
      // UNIVERSAL DUAL-SEGMENT RELOCATION ENGINE
      // ============================================================================
      const uint32_t iramStart = sizeof(MDBHeader); // 16
      const uint32_t iramEnd   = iramStart + header.iramSize;
      const uint32_t dramStart = iramEnd;
      const uint32_t dramEnd   = dramStart + header.dramSize;

      // --- PHASE 1: SCAN AND PATCH FULL IRAM LITERAL POOL ---
      uint32_t* literalPool = (uint32_t*)iramStagingArea;
      size_t literalWordCount = header.iramSize / 4;

#ifdef SERIAL_DEBUGGER
      Serial.printf("[MDB LOADER] Scanning IRAM literals up to size: %d bytes...\n\r", header.iramSize);
#endif

      for (size_t i = 0; i < literalWordCount; i++) {
          uint32_t rawVal = literalPool[i];
          uint32_t patchedAddr = 0;
          const char* targetSegment = nullptr;

          if ((rawVal & 3) == 0) {
              if (rawVal >= iramStart && rawVal < iramEnd) {
                  patchedAddr = (rawVal - iramStart) + (uint32_t)localIramBuffer;
                  targetSegment = "IRAM";
              } else if (rawVal >= dramStart && rawVal <= dramEnd) {
                  patchedAddr = (rawVal - dramStart) + (uint32_t)localDramBuffer;
                  targetSegment = "DRAM";
              }
          }

          if (targetSegment) {
              literalPool[i] = patchedAddr;
#ifdef SERIAL_DEBUGGER
       //        Serial.printf("  Pool [%d] @ 0x%04X (Raw: 0x%08X) -> 🛠️  PATCHED %s: 0x%08X\n\r",
       //                     i, (i * 4) + iramStart, rawVal, targetSegment, patchedAddr);
#endif
          }
      }
      memcpy(localIramBuffer, iramStagingArea, header.iramSize);
      free(iramStagingArea);

      // --- PHASE 2: SCAN AND PATCH GLOBAL OFFSET TABLE (GOT) ---
#ifdef SERIAL_DEBUG
      Serial.println("\nScanning Global Offset Table (GOT) Entries...");
#endif
      uint32_t gotDramOffset = header.gotFileOffset - iramEnd;

      if (gotDramOffset < header.dramSize) {
          uint32_t* realGotTable = (uint32_t*)(localDramBuffer + gotDramOffset);

          size_t remainingDramBytes = header.dramSize - gotDramOffset;
          size_t gotWordCount = remainingDramBytes / 4;

          if (gotWordCount > 256) gotWordCount = 256;

#ifdef SERIAL_DEBUG
          Serial.printf("  [MDB LOADER] Processing %d GOT entries safely...\n\r", gotWordCount);
#endif

          for (size_t i = 0; i < gotWordCount; i++) {
              uint32_t rawGot = realGotTable[i];

              if (rawGot >= iramStart && rawGot < iramEnd) {
                  realGotTable[i] = (rawGot - iramStart) + (uint32_t)localIramBuffer;
#ifdef SERIAL_DEBUG
                  Serial.printf("  GOT [%d] (Raw: 0x%08X) -> 🛠️  PATCHED IRAM: 0x%08X\n\r", i, rawGot, realGotTable[i]);
#endif
              } else if (rawGot >= dramStart && rawGot <= dramEnd) {
                  realGotTable[i] = (rawGot - dramStart) + (uint32_t)localDramBuffer;
#ifdef SERIAL_DEBUG
                  Serial.printf("  GOT [%d] (Raw: 0x%08X) -> 🛠️  PATCHED DRAM: 0x%08X\n\r", i, rawGot, realGotTable[i]);
#endif
              }
          }
      } else {
#ifdef SERIAL_DEBUG
          Serial.println("  [MDB WARN] gotDramOffset out of bounds, skipping GOT patch phase.");
#endif
      }

#ifdef SERIAL_DEBUG
      // ============================================================================
      // DIAGNOSTIC BLOCK: FIRST 32 BYTES OF DRAM (RAW CHARACTER VIEW)
      // ============================================================================
      Serial.printf("\n\rFirst 32 Bytes of DRAM Payload (Raw Character Look):\n\r");
      for (int i = 0; i < 32; i += 4) {
          Serial.printf("  DRAM + 0x%02X: %02X %02X %02X %02X | %c%c%c%c\n\r",
                        i,
                        localDramBuffer[i], localDramBuffer[i+1], localDramBuffer[i+2], localDramBuffer[i+3],
                        (localDramBuffer[i] >= 32 && localDramBuffer[i] <= 126) ? localDramBuffer[i] : '.',
                        (localDramBuffer[i+1] >= 32 && localDramBuffer[i+1] <= 126) ? localDramBuffer[i+1] : '.',
                        (localDramBuffer[i+2] >= 32 && localDramBuffer[i+2] <= 126) ? localDramBuffer[i+2] : '.',
                        (localDramBuffer[i+3] >= 32 && localDramBuffer[i+3] <= 126) ? localDramBuffer[i+3] : '.');
      }
      Serial.println("----------------------------------------\n\r");
#endif
      // ============================================================================
      // DISPATCH EXECUTION TASK
      // ============================================================================
      size_t alignedDramSize = (header.dramSize + 3) & ~3;

      ExecTaskParams* localTParams = (ExecTaskParams*)malloc(sizeof(ExecTaskParams));
      char** dramArgvArray = (char**)(localDramBuffer + alignedDramSize);
      char* dramArgsStringsPtr = (char*)(localDramBuffer + alignedDramSize + argvArraySize);

      for (int i = 0; i < argc; i++) {
          dramArgvArray[i] = dramArgsStringsPtr;
          strcpy(dramArgsStringsPtr, argTokens[i]);
          dramArgsStringsPtr += (strlen(argTokens[i]) + 1);
      }
      dramArgvArray[argc] = NULL;

      volatile bool binaryIsRunning = true;
      int appExitStatus = 0;
      TaskHandle_t binaryTaskHandle = NULL;

      localTParams->codeBuffer   = localIramBuffer;
      localTParams->dataBuffer   = localDramBuffer;
      localTParams->api          = &kernelAPI;
      localTParams->isRunning    = &binaryIsRunning;
      localTParams->argc         = argc;
      localTParams->argv         = dramArgvArray;
      localTParams->exitCode     = &appExitStatus;
      localTParams->entryOffset  = header.entryOffset;
      localTParams->dramSize     = header.dramSize;

      size_t totalTaskStackDepthWords = 2048 * 2;
      if (header.iramSize + header.dramSize > 512)  totalTaskStackDepthWords = 2048 * 3;
      if (header.iramSize + header.dramSize > 2048) totalTaskStackDepthWords = 2048 * 4;
      if (header.iramSize + header.dramSize > 4096) totalTaskStackDepthWords = 2048 * 5;
      if (header.iramSize + header.dramSize > 8192) totalTaskStackDepthWords = 2048 * 6;

      BaseType_t taskCreated = xTaskCreatePinnedToCore(
          nativeBinaryTaskWorker,
          "NativeBinTask",
          totalTaskStackDepthWords,
          localTParams,
          1,
          &binaryTaskHandle,
          1
      );

      while (true) {
          if (binaryTaskHandle != NULL && eTaskGetState(binaryTaskHandle) == eDeleted) {
              binaryTaskHandle = NULL;
              break;
          }

          if (!binaryIsRunning) {
              vTaskDelay(pdMS_TO_TICKS(150));
              break;
          }

          if (digitalRead(0) == LOW) {
              if (binaryTaskHandle != NULL) {
                  vTaskDelete(binaryTaskHandle);
                  vTaskDelay(pdMS_TO_TICKS(50));
                  binaryTaskHandle = NULL;
              }
              appExitStatus = -1;
              break;
          }
          vTaskDelay(pdMS_TO_TICKS(10));
      }

      if (localIramBuffer != NULL) { heap_caps_free(localIramBuffer); localIramBuffer = NULL; }
      if (localDramBuffer != NULL) { free(localDramBuffer);           localDramBuffer = NULL; }
      if (localTParams != NULL)    { free(localTParams);              localTParams = NULL; }
      return;
    } else {
      terminalPrintln("ERR: SPECIFY BINARY");
    }
    return;
  }
  else if (strcmp(firstToken, "NEW") == 0) {
    clearProgram();
    kernelAPI.clear();
    redrawTerminal();
    printLogo();
    return;
  }
  else if (strcmp(firstToken, "FREE") == 0) {
    uint32_t freeRam = ESP.getFreeHeap();
    char ramBuf[48];
    snprintf(ramBuf, 48, "FREE RAM: %u BYTES", freeRam);
    terminalPrintln(ramBuf);
    snprintf(ramBuf, 48, "(%u KB AVAILABLE)", freeRam / 1024);
    terminalPrintln(ramBuf);
    return;
  }
  else if (strcmp(firstToken, "HELP") == 0) {
    terminalPrintln("SYS:  FREE, HELP, LIST, NEW, RUN");
    terminalPrintln("DOS:  DIR, LOAD, LOAD$, SAVE, SAVE$, DELETE");
    terminalPrintln("CODE: BEEP, CIRCLE, CLEAR, COLOR, DELAY, DUMP,");
    terminalPrintln("      DUMPS, EDIT, EXEC, GOSUB, GOTO, HIGH, IF,");
    terminalPrintln("      IMVIEW, INKEY, INPUT, INREAD, IOSET, KEY,");
    terminalPrintln("      LINE, LOW, PEEK, PLOT, POKE, PRINT, RECT, ");
    terminalPrintln("      RETURN, RND, TOUCH, WIFIUP, WIFIDOWN");
    return;
  }
  else {
    char lookupName[FILENAME_SIZE];
    strncpy(lookupName, firstToken, FILENAME_SIZE);
    lookupName[FILENAME_SIZE - 1] = '\0';
    trimCString(lookupName);

    if (sdAvailable && strlen(lookupName) > 0) {
      char checkPath[FILENAME_SIZE];

      snprintf(checkPath, FILENAME_SIZE, "/%s.BIN", lookupName);
      if (SD.exists(checkPath)) {
        snprintf(cmd, MAX_LINE_LEN, "EXEC %s%s", checkPath, (firstSpace != NULL ? firstSpace : ""));
        cmd[MAX_LINE_LEN - 1] = '\0';
        continue;
      }

      snprintf(checkPath, FILENAME_SIZE, "/%s.BAS", lookupName);
      if (SD.exists(checkPath)) {
        snprintf(cmd, MAX_LINE_LEN, "RUN %s", checkPath);
        cmd[MAX_LINE_LEN - 1] = '\0';
        continue;
      }
    }
    int dummyIdx = 0;
    if (!runSingleLine(cmd, dummyIdx)) {
      terminalPrintln("UNKNOWN COMMAND OR FILE");
    }
    return;
  }
 }
}

bool runSingleLine(const char* rawLine, int &currentLineIdx) {
  char line[MAX_LINE_LEN];
  strncpy(line, rawLine, MAX_LINE_LEN);
  line[MAX_LINE_LEN - 1] = '\0';
  trimCString(line);

  int totalLen = strlen(line);
  if (totalLen == 0) return true;

  // ==========================================
  // 1. UNIFIED PRINT ENGINE
  // ==========================================
  if (strncmp(line, "PRINT ", 6) == 0 || strncmp(line, "PRINT(", 6) == 0) {
    char* payload = line + 6;
    if (line[5] == '(') {
      char* rParen = rindex(payload, ')');
      if (rParen) *rParen = '\0';
    }
    trimCString(payload);

    int startIdx = 0;
    int payloadLen = strlen(payload);
    while (startIdx < payloadLen) {
      int nextDelim = -1;
      bool insideQuotes = false;
      for (int i = startIdx; i < payloadLen; i++) {
        if (payload[i] == '"') insideQuotes = !insideQuotes;
        if (!insideQuotes && (payload[i] == ',' || payload[i] == ';')) {
          nextDelim = i;
          break;
        }
      }

      int endIdx = (nextDelim == -1) ? payloadLen : nextDelim;

      char token[MAX_LINE_LEN];
      int tokenChunk = endIdx - startIdx;
      if (tokenChunk >= (int)MAX_LINE_LEN) tokenChunk = MAX_LINE_LEN - 1;
      strncpy(token, payload + startIdx, tokenChunk);
      token[tokenChunk] = '\0';
      trimCString(token);
      int tLen = strlen(token);

      if (tLen > 0 && token[tLen - 1] == '$') {
        kernelAPI.print(getStringVariable(token));
      }
      else if (tLen >= 2 && token[0] == '"' && token[tLen - 1] == '"') {
        token[tLen - 1] = '\0';
        kernelAPI.print(token + 1);
      }
      else if (tLen > 0) {
        char valBuf[16];
        snprintf(valBuf, 16, "%d", evaluateExpression(token));
        kernelAPI.print(valBuf);
      }

      if (nextDelim != -1 && payload[nextDelim] == ',') {
        kernelAPI.print("    ");
      }

      startIdx = endIdx + 1;
    }

    kernelAPI.println("");
    return true;
  }

  // ==========================================
  // 2. CONTROL FLOW & BRANCH PRIMITIVES
  // ==========================================
  if (strncmp(line, "GOTO ", 5) == 0) {
    int targetLine = atoi(line + 5);
    for (int i = 0; i < programLineCount; i++) {
      if (programMemory[i].lineNumber == targetLine) {
        currentLineIdx = i;
        return true;
      }
    }
    memset(errBuf, 0, TERM_COLS);
    snprintf(errBuf, TERM_COLS, "ERR: GOTO TARGET MISSING %d", targetLine);
    kernelAPI.println(errBuf);
    return false;
  }

  if (strncmp(line, "GOSUB ", 6) == 0) {
    int targetLine = atoi(line + 6);
    if (stackPointer >= MAX_STACK_DEPTH) {
      kernelAPI.println("ERR: STACK OVERFLOW");
      return false;
    }

    subroutineCallStack[stackPointer] = currentLineIdx;
    stackPointer++;

    for (int i = 0; i < programLineCount; i++) {
      if (programMemory[i].lineNumber == targetLine) {
        currentLineIdx = i;
        return true;
      }
    }
    memset(errBuf, 0, TERM_COLS);
    snprintf(errBuf, TERM_COLS, "ERR: GOSUB TARGET MISSING %d", targetLine);
    kernelAPI.println(errBuf);
    return false;
  }

  if (strcmp(line, "RETURN") == 0) {
    if (stackPointer <= 0) {
      kernelAPI.println("ERR: RETURN WITHOUT GOSUB");
      return false;
    }
    stackPointer--;
    currentLineIdx = subroutineCallStack[stackPointer];
    return true;
  }

  if (strcmp(line, "END") == 0) { currentLineIdx = programLineCount; return true; }

  // ==========================================
  // 3. UNIFIED SOUND & TIMING DIRECTIVES
  // ==========================================
  if (strncmp(line, "DELAY ", 6) == 0 || strncmp(line, "PAUSE ", 6) == 0) {
    char* arg = line + 6;
    trimCString(arg);
    int ms = evaluateExpression(arg);
    if (ms > 0) {
      kernelAPI.delay((int)ms);
    }
    return true;
  }

  if (strncmp(line, "BEEP ", 5) == 0) {
    char* args = line + 5;
    char* commaIdx = strchr(args, ',');
    if (commaIdx != NULL) {
      *commaIdx = '\0';
      int freq = evaluateExpression(args);
      int duration = evaluateExpression(commaIdx + 1);
      if (freq > 0 && duration > 0) {
        kernelAPI.beep(freq, duration);
        return true;
      }
    }
    kernelAPI.println("ERR: INVALID BEEP ARGS");
    return false;
  }

  // ==========================================
  // 4. UNIFIED MEMORY STRUCT ADRESSING
  // ==========================================
  if (strncmp(line, "POKE ", 5) == 0) {
    char* args = line + 5;
    char* commaIdx = strchr(args, ',');
    if (commaIdx != NULL) {
      *commaIdx = '\0';
      int address = evaluateExpression(args);
      int value = evaluateExpression(commaIdx + 1);

      kernelAPI.poke(address, value);
      return true;
    }
    kernelAPI.println("ERR: INVALID POKE ARGS");
    return false;
  }

  // ==========================================
  // 5. UNIFIED GRAPHICS REDRAW MATRIX PIPELINES
  // ==========================================
  if (strncmp(line, "COLOR ", 6) == 0) {
    char* arg = line + 6;
    trimCString(arg);
    int colorId = evaluateExpression(arg);

    kernelAPI.color(colorId);
    return true;
  }

  if (strncmp(line, "RECT ", 5) == 0) {
    char* args = line + 5;
    char* p1 = strchr(args, ','); if (!p1) { kernelAPI.println("ERR: RECT SYNTAX"); return false; } *p1 = '\0';
    char* p2 = strchr(p1 + 1, ','); if (!p2) { kernelAPI.println("ERR: RECT SYNTAX"); return false; } *p2 = '\0';
    char* p3 = strchr(p2 + 1, ','); if (!p3) { kernelAPI.println("ERR: RECT SYNTAX"); return false; } *p3 = '\0';
    char* p4 = strchr(p3 + 1, ','); if (!p4) { kernelAPI.println("ERR: RECT SYNTAX"); return false; } *p4 = '\0';

    int x       = evaluateExpression(args);
    int y       = evaluateExpression(p1 + 1);
    int w       = evaluateExpression(p2 + 1);
    int h       = evaluateExpression(p3 + 1);
    int colorId = evaluateExpression(p4 + 1);

    kernelAPI.rect(x, y, w, h, colorId);
    return true;
  }

  if (strncmp(line, "CIRCLE ", 7) == 0) {
    char* args = line + 7;
    char* p1 = strchr(args, ','); if (!p1) { kernelAPI.println("ERR: CIRCLE SYNTAX"); return false; } *p1 = '\0';
    char* p2 = strchr(p1 + 1, ','); if (!p2) { kernelAPI.println("ERR: CIRCLE SYNTAX"); return false; } *p2 = '\0';
    char* p3 = strchr(p2 + 1, ','); if (!p3) { kernelAPI.println("ERR: CIRCLE SYNTAX"); return false; } *p3 = '\0';

    int x       = evaluateExpression(args);
    int y       = evaluateExpression(p1 + 1);
    int r       = evaluateExpression(p2 + 1);
    int colorId = evaluateExpression(p3 + 1);

    kernelAPI.circle(x, y, r, colorId);
    return true;
  }

  if (strncmp(line, "LINE ", 5) == 0) {
    char* args = line + 5;
    char* p1 = strchr(args, ','); if (!p1) { kernelAPI.println("ERR: LINE SYNTAX"); return false; } *p1 = '\0';
    char* p2 = strchr(p1 + 1, ','); if (!p2) { kernelAPI.println("ERR: LINE SYNTAX"); return false; } *p2 = '\0';
    char* p3 = strchr(p2 + 1, ','); if (!p3) { kernelAPI.println("ERR: LINE SYNTAX"); return false; } *p3 = '\0';
    char* p4 = strchr(p3 + 1, ','); if (!p4) { kernelAPI.println("ERR: LINE SYNTAX"); return false; } *p4 = '\0';

    int x1      = evaluateExpression(args);
    int y1      = evaluateExpression(p1 + 1);
    int x2      = evaluateExpression(p2 + 1);
    int y2      = evaluateExpression(p3 + 1);
    int colorId = evaluateExpression(p4 + 1);

    kernelAPI.line(x1, y1, x2, y2, colorId);
    return true;
  }

  if (strncmp(line, "PLOT ", 5) == 0) {
    char* args = line + 5;
    char* firstComma = strchr(args, ',');
    if (firstComma != NULL) {
      *firstComma = '\0';
      char* secondComma = strchr(firstComma + 1, ',');
      if (secondComma != NULL) {
        *secondComma = '\0';
        int x       = evaluateExpression(args);
        int y       = evaluateExpression(firstComma + 1);
        int colorId = evaluateExpression(secondComma + 1);

        kernelAPI.plot(x, y, colorId);
        return true;
      }
    }
    kernelAPI.println("ERR: INVALID PLOT ARGS");
    return false;
  }

  if (strcmp(line, "CLEAR") == 0 || strcmp(line, "CLS") == 0) {
    kernelAPI.clear();
    return true;
  }

  // ==========================================
  // 6. UNIFIED UI KEYBOARD AND DATA INPUT
  // ==========================================
  if (strncmp(line, "INKEY ", 6) == 0) {
    char varTarget[MAX_LINE_LEN];
    strncpy(varTarget, line + 6, MAX_LINE_LEN);
    varTarget[MAX_LINE_LEN - 1] = '\0';
    trimCString(varTarget);

    if (strlen(varTarget) > 0) {
      char foundKey = (char)kernelAPI.inkey();
      int targetLen = strlen(varTarget);

      if (varTarget[targetLen - 1] == '$') {
        char kStr[2] = { foundKey, '\0' };
        setStringVariable(varTarget, kStr);
      } else {
        setVariable(varTarget, (int)foundKey);
      }
      return true;
    }
    kernelAPI.println("ERR: INVALID INKEY ARGS");
    return false;
  }

  if (strncmp(line, "INPUT ", 6) == 0 || strncmp(line, "KEY ", 4) == 0) {
    bool read_key = false;
    char args[MAX_LINE_LEN];
    if (strncmp(line, "KEY ", 4) == 0) {
      read_key = true;
      strncpy(args, line + 4, MAX_LINE_LEN);
    } else {
      strncpy(args, line + 6, MAX_LINE_LEN);
    }
    args[MAX_LINE_LEN - 1] = '\0';
    trimCString(args);

    char promptStr[PROMPT_SIZE] = "";
    char varTarget[MAX_LINE_LEN] = "";
    bool withNewline = false;

    char* commaIdx = strrchr(args, ',');
    char* scolanIdx = strrchr(args, ';');
    if (commaIdx != NULL) {
      withNewline = true;
      *commaIdx = '\0';
      strncpy(promptStr, args, PROMPT_SIZE);
      strncpy(varTarget, commaIdx + 1, MAX_LINE_LEN);
    } else if (scolanIdx != NULL) {
      withNewline = false;
      *scolanIdx = '\0';
      strncpy(promptStr, args, PROMPT_SIZE);
      strncpy(varTarget, scolanIdx + 1, MAX_LINE_LEN);
    } else {
      strncpy(varTarget, args, MAX_LINE_LEN);
    }
    trimCString(promptStr);
    trimCString(varTarget);

    int pLen = strlen(promptStr);
    if (pLen >= 2 && promptStr[0] == '"' && promptStr[pLen - 1] == '"') {
      promptStr[pLen - 1] = '\0';
      memmove(promptStr, promptStr + 1, pLen - 1);
    }

    if (strlen(varTarget) > 0) {
      char targetBuffer[INPUT_BUF_SIZE] = "";

      if (read_key) {
        char k = 0;
        while (k == 0) {
            k = (char)kernelAPI.inkey();
            delay(10);
        }
        targetBuffer[0] = k;
        targetBuffer[1] = '\0';
      }
      else {
        if (strlen(promptStr) > 0 && withNewline) {
            kernelAPI.println(promptStr);
            kernelAPI.inputStr("", targetBuffer, INPUT_BUF_SIZE);
        } else {
            kernelAPI.inputStr(strlen(promptStr) > 0 ? promptStr : NULL, targetBuffer, INPUT_BUF_SIZE);
        }
      }

      int vLen = strlen(varTarget);
      if (varTarget[vLen - 1] == '$') {
        setStringVariable(varTarget, targetBuffer);
      } else {
        setVariable(varTarget, evaluateExpression(targetBuffer));
      }
      return true;
    }

    kernelAPI.println("ERR: INVALID INPUT ARGS");
    return false;
  }

  // ==========================================
  // 7. MAPPED INTERPRETER LOGIC
  // ==========================================
  if (strncmp(line, "IF ", 3) == 0) {
    char upperLine[MAX_LINE_LEN];
    strncpy(upperLine, line, MAX_LINE_LEN);
    upperLine[MAX_LINE_LEN - 1] = '\0';
    for (int i = 0; upperLine[i]; i++) upperLine[i] = toupper((unsigned char)upperLine[i]);

    char* splitPtr = strstr(upperLine, "THEN");
    bool isThenSplit = (splitPtr != NULL);
    if (!isThenSplit) {
      splitPtr = strstr(upperLine, "GOTO");
    }

    if (splitPtr != NULL) {
      int splitOffset = splitPtr - upperLine;

      char expression[MAX_LINE_LEN];
      strncpy(expression, line + 3, splitOffset - 3);
      expression[splitOffset - 3] = '\0';
      trimCString(expression);

      char trailingAction[MAX_LINE_LEN];
      strncpy(trailingAction, line + splitOffset + 4, MAX_LINE_LEN);
      trailingAction[MAX_LINE_LEN - 1] = '\0';
      trimCString(trailingAction);

      enum Op { EQUAL, NOT_EQUAL, GREATER, LESS, G_EQUAL, L_EQUAL, NONE };
      Op foundOp = NONE; int opPos = -1; int opLen = 1;

      char* opSearch;
      if ((opSearch = strstr(expression, "==")) != NULL) { foundOp = EQUAL; opPos = opSearch - expression; opLen = 2; }
      else if ((opSearch = strstr(expression, "!=")) != NULL) { foundOp = NOT_EQUAL; opPos = opSearch - expression; opLen = 2; }
      else if ((opSearch = strstr(expression, ">=")) != NULL) { foundOp = G_EQUAL; opPos = opSearch - expression; opLen = 2; }
      else if ((opSearch = strstr(expression, "<=")) != NULL) { foundOp = L_EQUAL; opPos = opSearch - expression; opLen = 2; }
      else if ((opSearch = strstr(expression, ">")) != NULL)  { foundOp = GREATER; opPos = opSearch - expression; }
      else if ((opSearch = strstr(expression, "<")) != NULL)  { foundOp = LESS; opPos = opSearch - expression; }
      else if ((opSearch = strstr(expression, "=")) != NULL)   { foundOp = EQUAL; opPos = opSearch - expression; }

      if (foundOp != NONE) {
        char leftSide[MAX_LINE_LEN];
        strncpy(leftSide, expression, opPos);
        leftSide[opPos] = '\0';
        trimCString(leftSide);

        char rightSide[MAX_LINE_LEN];
        strncpy(rightSide, expression + opPos + opLen, MAX_LINE_LEN);
        rightSide[MAX_LINE_LEN - 1] = '\0';
        trimCString(rightSide);

        bool conditionMet = false;
        int lLen = strlen(leftSide);
        int rLen = strlen(rightSide);

        if (leftSide[lLen - 1] == '$' || rightSide[rLen - 1] == '$' || leftSide[0] == '"' || rightSide[0] == '"') {
          char leftStr[MAX_STR_LEN] = "";
          if (leftSide[lLen - 1] == '$') {
            strncpy(leftStr, getStringVariable(leftSide), MAX_STR_LEN);
          } else if (leftSide[0] == '"' && leftSide[lLen - 1] == '"') {
            strncpy(leftStr, leftSide + 1, lLen - 2);
            leftStr[lLen - 2] = '\0';
          }

          char rightStr[MAX_STR_LEN] = "";
          if (rightSide[rLen - 1] == '$') {
            strncpy(rightStr, getStringVariable(rightSide), MAX_STR_LEN);
          } else if (rightSide[0] == '"' && rightSide[rLen - 1] == '"') {
            strncpy(rightStr, rightSide + 1, rLen - 2);
            rightStr[rLen - 2] = '\0';
          }

          if (foundOp == EQUAL) conditionMet = (strcmp(leftStr, rightStr) == 0);
          else if (foundOp == NOT_EQUAL) conditionMet = (strcmp(leftStr, rightStr) != 0);
          else {
            kernelAPI.println("ERR: STRING ONLY SUPPORTS == AND !=");
            return false;
          }
        }
        else {
          int leftVal = evaluateExpression(leftSide);
          int rightVal = evaluateExpression(rightSide);
          switch (foundOp) {
            case EQUAL: conditionMet = (leftVal == rightVal); break;
            case NOT_EQUAL: conditionMet = (leftVal != rightVal); break;
            case GREATER: conditionMet = (leftVal > rightVal); break;
            case LESS: conditionMet = (leftVal < rightVal); break;
            case G_EQUAL: conditionMet = (leftVal >= rightVal); break;
            case L_EQUAL: conditionMet = (leftVal <= rightVal); break;
            default: break;
          }
        }

        if (conditionMet) {
          bool isPureNumericJump = true;
          for (int i = 0; trailingAction[i]; i++) {
            if (!isdigit((unsigned char)trailingAction[i])) { isPureNumericJump = false; break; }
          }

          if (isPureNumericJump && strlen(trailingAction) > 0) {
            int targetLine = atoi(trailingAction);
            for (int i = 0; i < programLineCount; i++) {
              if (programMemory[i].lineNumber == targetLine) { currentLineIdx = i; return true; }
            }
            memset(errBuf, 0, TERM_COLS);
            snprintf(errBuf, TERM_COLS, "ERR: LINE MISSING %d", targetLine);
            kernelAPI.println(errBuf);
            return false;
          }

          if (strncmp(trailingAction, "GOTO ", 5) == 0) {
            int targetLine = atoi(trailingAction + 5);
            for (int i = 0; i < programLineCount; i++) {
              if (programMemory[i].lineNumber == targetLine) { currentLineIdx = i; return true; }
            }
            memset(errBuf, 0, TERM_COLS);
            snprintf(errBuf, TERM_COLS, "ERR: LINE MISSING %d", targetLine);
            kernelAPI.println(errBuf);
            return false;
          }

          return runSingleLine(trailingAction, currentLineIdx);
        }
        return true;
      }
    }
    kernelAPI.println("SYNTAX ERROR: INVALID IF");
    return false;
  }

  // ==========================================
  // 8. HARDWARE CONTROLS (IOSET, HIGH, LOW)
  // ==========================================
  if (strncmp(line, "IOSET ", 6) == 0) {
    char* args = line + 6;
    char* commaIdx = strchr(args, ',');
    if (commaIdx != NULL) {
      *commaIdx = '\0';
      int pin = evaluateExpression(args);
      int mode = evaluateExpression(commaIdx + 1);

      kernelAPI.pinMode(pin, mode == 1 ? OUTPUT : INPUT);
      return true;
    }
    kernelAPI.println("ERR: INVALID IOSET ARGS");
    return false;
  }

  if (strncmp(line, "HIGH ", 5) == 0) {
    char* arg = line + 5;
    trimCString(arg);
    int pin = evaluateExpression(arg);

    kernelAPI.digitalWrite(pin, HIGH);
    return true;
  }

  if (strncmp(line, "LOW ", 4) == 0) {
    char* arg = line + 4;
    trimCString(arg);
    int pin = evaluateExpression(arg);

    kernelAPI.digitalWrite(pin, LOW);
    return true;
  }

  // ==========================================
  // WI-FI MANAGEMENT ACTIONS
  // ==========================================
  if (strncmp(line, "WIFIUP ", 7) == 0) {
    char* args = line + 7;
    char* commaIdx = strchr(args, ',');
    if (commaIdx != NULL) {
      *commaIdx = '\0';
      char ssid[16];
      char pass[16];
      strncpy(ssid, args, 16); ssid[16-1] = '\0';
      strncpy(pass, commaIdx + 1, 16); pass[16-1] = '\0';
      trimCString(ssid); trimCString(pass);

      int sLen = strlen(ssid);
      if (sLen >= 2 && ssid[0] == '"' && ssid[sLen - 1] == '"') { ssid[sLen - 1] = '\0'; memmove(ssid, ssid + 1, sLen - 1); }
      int pLen = strlen(pass);
      if (pLen >= 2 && pass[0] == '"' && pass[pLen - 1] == '"') { pass[pLen - 1] = '\0'; memmove(pass, pass + 1, pLen - 1); }

      kernelAPI.println("PROVISIONING INTERNAL NETWORK CARD...");

      if (kernelAPI.wifiUp(ssid, pass) == 0) {
        IPAddress localIP = WiFi.localIP();
        char ipBuf[32];
        snprintf(ipBuf, 32, "NETWORK IS LIVE. IP: %d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);
        kernelAPI.println(ipBuf);
      } else {
        kernelAPI.println("ERR: CONNECTION TIMEOUT OR BAD CREDENTIALS");
      }
      return true;
    }
    kernelAPI.println("ERR: INVALID WIFIUP ARGS (SSID,PASS)");
    return false;
  }

  if (strcmp(line, "WIFIDOWN") == 0) {
    kernelAPI.println("SHUTTING DOWN INTERNAL RADIO...");
    kernelAPI.wifiDown();
    kernelAPI.println("RADIO CORES OFFLINE.");
    return true;
  }

  // ==========================================
  // FOR LOOP INITIALIZATION
  // ==========================================
  if (strncmp(line, "FOR ", 4) == 0) {
    char* args = line + 4;
    trimCString(args);

    char* eqIdx = strchr(args, '=');
    char* toIdx = strstr(args, " TO ");

    if (eqIdx != NULL && toIdx != NULL) {
      *eqIdx = '\0';
      *toIdx = '\0';
      char* varStr = args;
      char* startStr = eqIdx + 1;
      char* endStr = toIdx + 4;
      trimCString(varStr); trimCString(startStr); trimCString(endStr);

      int stepValue = 1;
      char* stepIdx = strstr(endStr, " STEP ");
      if (stepIdx != NULL) {
        *stepIdx = '\0';
        stepValue = evaluateExpression(stepIdx + 6);
        trimCString(endStr);
      }

      if (strlen(varStr) == 1 && isAlpha((unsigned char)varStr[0])) {
        char loopVar = varStr[0];
        int startVal = evaluateExpression(startStr);
        int targetVal = evaluateExpression(endStr);

        setVariable(varStr, startVal);

        if (forStackPointer >= MAX_FOR_NEST) {
          kernelAPI.println("ERR: FOR LOOP NEST OVERFLOW");
          return false;
        }

        forStack[forStackPointer].varName = loopVar;
        forStack[forStackPointer].targetValue = targetVal;
        forStack[forStackPointer].stepValue = stepValue;
        forStack[forStackPointer].lineMemoryIdx = currentLineIdx + 1;
        forStackPointer++;
        return true;
      }
    }
    kernelAPI.println("SYNTAX ERROR: INVALID FOR LOOP");
    return false;
  }
	
  // ==========================================
  // NEXT LOOP EVALUATION
  // ==========================================
  if (strncmp(line, "NEXT", 4) == 0) {
    char varStr[MAX_VAR_NAME_LEN];
    strncpy(varStr, line + 4, MAX_VAR_NAME_LEN);
    varStr[MAX_VAR_NAME_LEN-1] = '\0';
    trimCString(varStr);
    if (forStackPointer <= 0) {
      kernelAPI.println("ERR: NEXT WITHOUT FOR");
      return false;
    }
    int activeIdx = forStackPointer - 1;
    if (strlen(varStr) == 1 && varStr[0] != forStack[activeIdx].varName) {
      kernelAPI.println("ERR: NEXT VARIABLE MISMATCH");
      return false;
    }
    char loopVar = forStack[activeIdx].varName;
    char varKey[2] = { loopVar, '\0' };
    int currentVal = getVariable(varKey);
    currentVal += forStack[activeIdx].stepValue;
    setVariable(varKey, currentVal);
    bool loopFinished = false;
    if (forStack[activeIdx].stepValue >= 0) {
      if (currentVal > forStack[activeIdx].targetValue) loopFinished = true;
    } else {
      if (currentVal < forStack[activeIdx].targetValue) loopFinished = true;
    }
    if (!loopFinished) {
      currentLineIdx = forStack[activeIdx].lineMemoryIdx;
    } else {
      forStackPointer--;
    }
    return true;
  }

  // ==========================================
  // 10. COMMENTS FILTER
  // ==========================================
  if (strncmp(line, "REM", 3) == 0) {
    return true;
  }

  // ==========================================
  // 11. VARIABLE ASSIGNMENT PIPELINE
  // ==========================================
  char* workLine = line;
  if (strncmp(workLine, "LET ", 4) == 0) {
    workLine += 4;
    trimCString(workLine);
  }

  char* eqIdx = strchr(workLine, '=');
  if (eqIdx != NULL) {
    *eqIdx = '\0';
    char* varName = workLine;
    char* rhs = eqIdx + 1;
    trimCString(varName); trimCString(rhs);
    int vNameLen = strlen(varName);

    if (vNameLen > 0 && isAlpha((unsigned char)varName[0])) {
      if (varName[vNameLen - 1] == '$') {
        int rLen = strlen(rhs);
        if (strncmp(rhs, "STR$(", 5) == 0 && rhs[rLen - 1] == ')') {
          rhs[rLen - 1] = '\0';
          char* innerExpr = rhs + 5;
          trimCString(innerExpr);
          int numericValue = evaluateExpression(innerExpr);
          char castBuffer[MAX_VAR_NAME_LEN];
          snprintf(castBuffer, MAX_VAR_NAME_LEN, "%d", numericValue);
          setStringVariable(varName, castBuffer);
          return true;
        }

        char* firstQuote = strchr(rhs, '"');
        char* lastQuote = strrchr(rhs, '"');

        if (firstQuote != NULL && lastQuote != NULL && firstQuote != lastQuote) {
          *lastQuote = '\0';
          char* strVal = firstQuote + 1;
          setStringVariable(varName, strVal);
          return true;
        }
        kernelAPI.println("ERR: STRING LITERAL OR STR$ EXPECTED");
        return false;
      }

      setVariable(varName, evaluateExpression(rhs));
      return true;
    }
  }
  kernelAPI.println("SYNTAX ERROR");
  return false;
}

void writeEscapedToStream(WiFiClient* stream, const char* src) {
  if (!src) return;
  while (*src) {
    if (*src == '\\') stream->print(F("\\\\"));
    else if (*src == '"') stream->print(F("\\\""));
    else if (*src == '\n') stream->print(F("\\n"));
    else if (*src == '\r') stream->print(F("\\r"));
    else if (*src == '\t') stream->print(F("\\t"));
    else stream->print(*src);
    src++;
  }
}

void appendToBuffer(char* dest, const char* src, size_t destMaxSize) {
    size_t curLen = strlen(dest);
    size_t srcLen = strlen(src);
    if (curLen + srcLen < destMaxSize) {
        strcpy(dest + curLen, src);
    }
}

void slideWindowAppend(char* dest, const char* src, size_t maxWindowSize) {
    size_t srcLen = strlen(src);
    size_t destLen = strlen(dest);
    if (destLen + srcLen < maxWindowSize) {
        strcpy(dest + destLen, src);
    } else {
        size_t totalLen = destLen + srcLen;
        size_t chopAmt = totalLen - (maxWindowSize - 1);
        if (chopAmt < destLen) {
            memmove(dest, dest + chopAmt, destLen - chopAmt + 1);
            strcpy(dest + strlen(dest), src);
        } else {
            strncpy(dest, src + (srcLen - (maxWindowSize - 1)), maxWindowSize - 1);
            dest[maxWindowSize - 1] = '\0';
        }
    }
}

void processIncomingToken(const char* token, const char* m, bool &isRecordingCode, char* textAccumulator, size_t textAccumMaxSize, char* markdownBuffer, size_t mdMaxSize, bool &insideThinkBlock, char* fullAssistantResponse, size_t farMaxSize, int streamToConsole) {
    const char* cleanToken = token;
    if (strcmp(token, "\\n") == 0) cleanToken = "\n";
    else if (strcmp(token, "\\t") == 0) cleanToken = "\t";
    else if (strcmp(token, "\\\"") == 0) cleanToken = "\"";
    else if (strcmp(token, "\\\\") == 0) cleanToken = "\\";

    // ==========================================
    // THINKING TAG FILTER ENGINE
    // ==========================================
    if (strstr(cleanToken, "<think>") != NULL) {
        insideThinkBlock = true;
        if (showThinkingLogs) terminalPrint("\n[THINKING: ");
        return;
    }
    if (strstr(cleanToken, "</think>") != NULL) {
        insideThinkBlock = false;
        if (showThinkingLogs) terminalPrintln("]\n");
        return;
    }
    if (insideThinkBlock) {
        if (showThinkingLogs && streamToConsole == 1) {
            if (strcmp(token, "\\n") == 0) terminalPrint(" ");
            else if (strcmp(token, "\\t") != 0) terminalPrint(cleanToken);
        }
        return;
    }

    appendToBuffer(fullAssistantResponse, cleanToken, farMaxSize);

    // ==========================================
    // 2. MARKDOWN EXTRACTION ENGINE (CODER ONLY)
    // ==========================================
    if (strstr(m, "coder") != NULL) {
        slideWindowAppend(markdownBuffer, cleanToken, mdMaxSize);

        if (!isRecordingCode) {
            if (strstr(markdownBuffer, "```basic") != NULL || strstr(markdownBuffer, "```BASIC") != NULL) {
                isRecordingCode = true;
                clearProgram();
                textAccumulator[0] = '\0';
                markdownBuffer[0] = '\0';
            }
        } else {
            if (strstr(markdownBuffer, "```") != NULL) {
                isRecordingCode = false;
                char* trimIdx = strstr(textAccumulator, "```");
                if (trimIdx != NULL) *trimIdx = '\0';

                char* startPos = textAccumulator;
                while (startPos != NULL && *startPos != '\0') {
                    char* nextNewline = strchr(startPos, '\n');
                    if (nextNewline != NULL) *nextNewline = '\0';

                    char codeLine[MAX_LINE_LEN];
                    strncpy(codeLine, startPos, MAX_LINE_LEN);
                    codeLine[MAX_LINE_LEN - 1] = '\0';
                    trimCString(codeLine);

                    if (strlen(codeLine) > 0) {
                        char* spaceIdx = strchr(codeLine, ' ');
                        if (spaceIdx != NULL) {
                            *spaceIdx = '\0';
                            bool isNum = true;
                            for (int i = 0; codeLine[i]; i++) {
                                if (!isdigit((unsigned char)codeLine[i])) { isNum = false; break; }
                            }
                            if (isNum) {
                                storeLine(atoi(codeLine), spaceIdx + 1);
                            }
                        }
                    }
                    if (nextNewline != NULL) startPos = nextNewline + 1;
                    else startPos = NULL;
                }
            } else {
                appendToBuffer(textAccumulator, cleanToken, textAccumMaxSize);
            }
        }
    }

    // ==========================================
    // 3. CONSOLE TEXT DRAWING ENGINE
    // ==========================================
    if (streamToConsole == 1) {
        if (strcmp(token, "\\n") == 0)      terminalPrintln("");
        else if (strcmp(token, "\\t") == 0) terminalPrint("    ");
        else                     terminalPrint(cleanToken);
    }
}

void api_setup() {
  kernelAPI.print   = [] (const char* t) {
    if (t) terminalPrint(t);
  };

  kernelAPI.println = [] (const char* t) {
    if (t) terminalPrintln(t);
  };

  kernelAPI.clear   = [] () {
    for(int i=0;i<TOTAL_ROWS;i++) memset(terminalBuffer[i], 0, TERM_COLS + 1);
    for(int i=0;i<TOTAL_ROWS;i++) memset(colorBuffer[i], 0, TERM_COLS);
    activeRowIndex=0;
    scrollOffset = 0;
    cursorX = 0;
    cursorY = 0;
    if (MATRIX_ACTIVE) {
      bgCanvas.fillRect(0,0,TFT_WIDTH,TFT_WIDTH,TFT_BLACK);
      bgCanvas.pushSprite(0, 0);
    } else {
      tft.fillRect(0,0,TFT_WIDTH,TFT_WIDTH,TFT_BLACK);
    }
    redrawTerminal();
  };

  kernelAPI.beep    = [] (int f, int d) {
     playBeep(f, d);
  };

  kernelAPI.delay   = [] (int ms) {
    delay((uint32_t)ms);
  };

  kernelAPI.inkey   = [] () -> int {
    char foundKey = getKeyPress(false);
    return (int)foundKey;
  };

  kernelAPI.color = [] (int colorId) {
    if (MATRIX_ACTIVE) {
      currentActivePaletteId = colorId;
    } else {
      currentActivePaletteId = colorId;
    }
  };

  kernelAPI.plot = [] (int x, int y, int colorId) {
    if (x >= 0 && x < TFT_WIDTH && y >= 0 && y < TFT_WIDTH) {
      if (MATRIX_ACTIVE) {
        bgCanvas.drawPixel(x, y, colorId);
      } else {
        tft.drawPixel(x, y, getPaletteColor(colorId));
      }
    }
  };

  kernelAPI.line = [] (int x1, int y1, int x2, int y2, int colorId) {
    if (x1 >= 0 && x1 < TFT_WIDTH && y1 >= 0 && y1 < TFT_WIDTH && x2 >= 0 && x2 < TFT_WIDTH && y2 >= 0 && y2 < TFT_WIDTH) {
      if (MATRIX_ACTIVE) {
        bgCanvas.drawLine(x1, y1, x2, y2, colorId);
      } else {
        tft.drawLine(x1, y1, x2, y2, getPaletteColor(colorId));
      }
    }
  };

  kernelAPI.rect = [] (int x, int y, int w, int h, int colorId) {
     if (MATRIX_ACTIVE) {
      bgCanvas.fillRect(x, y, w, h, colorId);
    } else {
      tft.fillRect(x, y, w, h, getPaletteColor(colorId));
    }
  };

  kernelAPI.circle = [] (int x, int y, int r, int colorId) {
    if (MATRIX_ACTIVE) {
      bgCanvas.fillCircle(x, y, r, colorId);
    } else {
      tft.fillCircle(x, y, r, getPaletteColor(colorId));
    }
  };

  kernelAPI.peek = [] (int address) -> int {
    if (address >= 0 && address < SYSTEM_RAM_SIZE) return systemRAM[address];
    return 0;
  };

  kernelAPI.poke = [] (int address, int value) {
    if (address >= 0 && address < SYSTEM_RAM_SIZE) {
      systemRAM[address] = value;
    }
  };

  kernelAPI.getRamSize = [] () -> int {
    return SYSTEM_RAM_SIZE;
  };

  kernelAPI.pinMode = [] (int pin, int mode) {
    if (pin != 5 && pin != 12 && pin != 13 && pin != 14 && pin != 15 && pin != 18 && pin != 19 && pin != 23) {
      pinMode(pin, mode);
    }
  };

  kernelAPI.digitalWrite = [] (int pin, int val) {
    if (pin != 5 && pin != 12 && pin != 13 && pin != 14 && pin != 15 && pin != 18 && pin != 19 && pin != 23) {
      digitalWrite(pin, val);
    }
  };

  kernelAPI.digitalRead = [] (int pin) -> int {
    return digitalRead(pin);
  };

  kernelAPI.inputStr = [] (const char* prompt, char* destBuffer, int maxLen) {
    if (!destBuffer || maxLen <= 0) return;
    strncpy(previousPrompt, currentPrompt, PROMPT_SIZE);
    if (prompt) {
      memset(currentPrompt, 0, PROMPT_SIZE);
      strncpy(currentPrompt, prompt, PROMPT_SIZE);
    } else {
      memset(currentPrompt, 0, PROMPT_SIZE);
    }

    if (currentPrompt && strlen(currentPrompt) > 0) {
      terminalPrint(currentPrompt);
      terminalPrint(cursor);
    } else {
      terminalPrint(cursor);
    }

    bool readingInput = true;
    memset(inputBuffer, 0, INPUT_BUF_SIZE);

    while (readingInput) {
      char pressedKey = getKeyPress(true);
      if (pressedKey > 0) {
        handleKeyPress(pressedKey);
        if (pressedKey == '\n') {
          uint32_t bytesToCopy = strlen(inputBuffer);

          if (bytesToCopy >= maxLen) bytesToCopy = maxLen - 1;

          memcpy(destBuffer, inputBuffer, bytesToCopy);
          destBuffer[bytesToCopy] = '\0';
          memset(inputBuffer, 0, INPUT_BUF_SIZE);

          readingInput = false;
        }
      }
      delay(10);
    }

    trimCString(destBuffer);

    strncpy(currentPrompt, previousPrompt, PROMPT_SIZE);
  };

  kernelAPI.wifiUp = [] (const char* ssid, const char* pass) -> int {
    char localSSID[16] = {0};
    char localPASS[16] = {0};
    bool useFallback = false;

    if (!ssid || !pass || strcmp(ssid, "SSID") == 0 || strcmp(pass, "PASS") == 0 || strlen(ssid) == 0) {
      useFallback = true;
    }

    if (useFallback) {
#ifdef SERIAL_DEBUGGER
      Serial.println("[SYS] Blank or placeholder credentials. Accessing SD fallback...");
#endif
      File wifiFile = SD.open("/WIFI.CFG", FILE_READ);
      if (wifiFile) {
        int len = wifiFile.readBytesUntil('\n', localSSID, sizeof(localSSID) - 1);
        localSSID[len] = '\0';

        len = wifiFile.readBytesUntil('\n', localPASS, sizeof(localPASS) - 1);
        localPASS[len] = '\0';

        wifiFile.close();
      } else {
#ifdef SERIAL_DEBUGGER
        Serial.println("[ERR] /WIFI.CFG not found!");
#endif
        return -1;
      }
    } else {
      strncpy(localSSID, ssid, sizeof(localSSID) - 1);
      localSSID[sizeof(localSSID) - 1] = '\0';

      strncpy(localPASS, pass, sizeof(localPASS) - 1);
      localPASS[sizeof(localPASS) - 1] = '\0';
    }

    trimCString(localSSID);
    trimCString(localPASS);

    if (strlen(localSSID) == 0) return -1;

#ifdef SERIAL_DEBUGGER
    Serial.printf("[SYS] Initiating Wi-Fi connection target: [%s]\n\r", localSSID);
#endif

    WiFi.mode(WIFI_STA);
    WiFi.begin(localSSID, localPASS);

    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(500);
      timeout++;
    }

    if (WiFi.status() == WL_CONNECTED) {
#ifdef SERIAL_DEBUGGER
      Serial.printf("[SYS] Wi-Fi connected! Station IP address: %s\n\r", WiFi.localIP().toString().c_str());
#endif
      return 0;
    } else {
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
      return -1;
    }
  };

  kernelAPI.wifiDown = [] () {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  };


  kernelAPI.ollamaStream = [] (const char* p, const char* s, const char* m, const char* sysPrompt, int streamToConsole) -> int {
    if (WiFi.status() != WL_CONNECTED || !p || strlen(p) == 0 || !m) return -1;

    String inputStr = String(p);
    String thinking = "false";

    inputStr.trim();
    inputStr.toUpperCase();

    if (inputStr == "") {
      terminalPrintln("");
      return 0;
    }
    if (inputStr == "RESET" || inputStr == "FORGET" || inputStr == "NEW") {
      clearConversationContext();
      terminalPrintln("System: Memory cleared. Fresh session started.");
      return 0;
    }
    if (inputStr == "THINK") {
      showThinkingLogs = true;
      terminalPrintln("System: Thinking logs enabled (VISIBLE).");
      return 0;
    }
    if (inputStr == "NOTHINK") {
      showThinkingLogs = false;
      terminalPrintln("System: Thinking logs disabled (HIDDEN).");
      return 0;
    }
    if (showThinkingLogs) {
      thinking = "true";
    }

    if (String(m).indexOf("coder") > -1) {
      activeContext = coderContext;
    } else {
      activeContext = chatContext;
    }


    if (strlen(activeContext) == 0) {
      snprintf(activeContext, CONTEXT_BUF_SIZE, "User: %s\nAssistant: ", p);
    } else {
      char newTurn[512];
      snprintf(newTurn, 512, "\nUser: %s\nAssistant: ", p);
      while ((strlen(activeContext) + strlen(newTurn)) >= CONTEXT_BUF_SIZE) {
        char* nextTurn = strstr(activeContext + 6, "User:");
        if (nextTurn) {
          memmove(activeContext, nextTurn, strlen(nextTurn) + 1);
        } else {
          activeContext[0] = '\0';
          break;
        }
      }
      strcat(activeContext, newTurn);
    }


    HTTPClient http;
    http.begin("http://" + String(s) + ":11434/api/generate");
    http.setTimeout((uint16_t)30000);
    http.addHeader("Content-Type", "application/json");

    String jsonPrompt = String(activeContext);
    jsonPrompt.replace("\\", "\\\\");
    jsonPrompt.replace("\"", "\\\"");
    jsonPrompt.replace("\n", "\\n");

    String systemInstructions = (sysPrompt && strlen(sysPrompt) > 0) ? String(sysPrompt) : "You are a witty multidisciplinary assistant.";

    systemInstructions.replace("\\", "\\\\");
    systemInstructions.replace("\"", "\\\"");
    systemInstructions.replace("\n", "\\n");

    String payload = "{"
          "\"model\":\"" + String(m) + "\","
          "\"prompt\":\"" + jsonPrompt + "\","
          "\"system\":\"" + systemInstructions + "\","
          "\"stream\":true,"
	  "\"think\":" + thinking + ","
          "\"options\":{"
            "\"temperature\":0.1,"
            "\"stop\":[\"User:\",\"\\nUser:\"]"
          "}"
        "}";

    int code = http.POST(payload);

    if (code == HTTP_CODE_OK) {
      WiFiClient* stream = http.getStreamPtr();

      bool isRecordingCode = false;

      #define WORK_ACC_SIZE 1536
      #define WORK_MD_SIZE 64
      #define WORK_FAR_SIZE 1024
      char* textAccumulator = (char*)malloc(WORK_ACC_SIZE);
      char markdownBuffer[WORK_MD_SIZE] = "";
      char* fullAssistantResponse = (char*)malloc(WORK_FAR_SIZE);
      if (textAccumulator) textAccumulator[0] = '\0';
      if (fullAssistantResponse) fullAssistantResponse[0] = '\0';

      bool insideThinkBlock = false;
      String responseWindow = "";
      bool insideResponseValue = false;
      bool escapeActive = false;

      while (http.connected() && (stream->available() || insideResponseValue)) {
        if (!stream->available()) {
          delay(1);
          continue;
        }

        char c = stream->read();

        if (!insideResponseValue) {
          responseWindow += c;
          if (responseWindow.length() > 14) {
            responseWindow = responseWindow.substring(responseWindow.length() - 14);
          }

          if (responseWindow.endsWith("\"response\":\"")) {
            insideResponseValue = true;
            escapeActive = false;
            responseWindow = "";
          }
          else if (responseWindow.endsWith("\"thinking\":\"")) {
            if (showThinkingLogs) {
              insideResponseValue = true;
              escapeActive = false;
            }
            responseWindow = "";
          }
          continue;
        }

        if (insideResponseValue) {
          if (escapeActive) {
            String token = "\\" + String(c);
            escapeActive = false;
            if (textAccumulator && fullAssistantResponse) {
              processIncomingToken(token.c_str(), m, isRecordingCode, textAccumulator,
			           WORK_ACC_SIZE, markdownBuffer, WORK_MD_SIZE, insideThinkBlock,
				   fullAssistantResponse, WORK_FAR_SIZE, streamToConsole);
            }
          }
          else if (c == '\\') {
            escapeActive = true;
          }
          else if (c == '"') {
            insideResponseValue = false;
          }
          else {
            String token = String(c);
            if (textAccumulator && fullAssistantResponse) {
              processIncomingToken(token.c_str(), m, isRecordingCode, textAccumulator, WORK_ACC_SIZE,
			            markdownBuffer, WORK_MD_SIZE, insideThinkBlock, fullAssistantResponse,
				    WORK_FAR_SIZE, streamToConsole);
            }
          }
        }
        delay(1);
      }

      if (fullAssistantResponse && strlen(fullAssistantResponse) > 0) {
        String farStr = String(fullAssistantResponse);
        if (farStr.endsWith("User:")) {
          farStr = farStr.substring(0, farStr.length() - 5);
        }

        while ((strlen(activeContext) + farStr.length()) >= CONTEXT_BUF_SIZE) {
          char* nextTurn = strstr(activeContext + 6, "User:");
          if (nextTurn) {
            memmove(activeContext, nextTurn, strlen(nextTurn) + 1);
          } else {
            activeContext[0] = '\0';
            break;
          }
        }
        strcat(activeContext, farStr.c_str());
      }

      if (textAccumulator) free(textAccumulator);
      if (fullAssistantResponse) free(fullAssistantResponse);
      http.end();
      return 0;
    }

    http.end();
    return -1;
  };

  kernelAPI.getTouch    = kernel_getTouchState;

  kernelAPI.termWidth   = TFT_WIDTH;

  kernelAPI.termHeight  = TFT_WIDTH;

  kernelAPI.charWidth   = CHAR_WIDTH;

  kernelAPI.charHeight  = CHAR_HEIGHT;

  kernelAPI.setFKeys = [] (const char* l1, const char* l2, const char* l3, const char* l4, const char* l5) {
    strlcpy(fKeyLabels[0], l1 ? l1 : "F1", F_KEY_LABEL_SIZE);
    strlcpy(fKeyLabels[1], l2 ? l2 : "F2", F_KEY_LABEL_SIZE);
    strlcpy(fKeyLabels[2], l3 ? l3 : "F3", F_KEY_LABEL_SIZE);
    strlcpy(fKeyLabels[3], l4 ? l4 : "F4", F_KEY_LABEL_SIZE);
    strlcpy(fKeyLabels[4], l5 ? l5 : "F5", F_KEY_LABEL_SIZE);

    fKeysOverlayActive = true;
    drawStatusBar();
  };

  kernelAPI.clearFKeys = [] () {
    fKeysOverlayActive = false;
    drawStatusBar();
  };

  kernelAPI.malloc = [] (unsigned int size) -> void* {
    return malloc(size);
  };

  kernelAPI.free   = [] (void* ptr) {
    if (ptr) free(ptr);
  };

  kernelAPI.createSprite = [](const char* filename) -> uint32_t {
    if (!filename || filename[0] == '\0' || !sdAvailable) return 0;

    char safePath[64];
    if (filename[0] != '/') {
      snprintf(safePath, sizeof(safePath), "/%s", filename);
    } else {
      strncpy(safePath, filename, sizeof(safePath) - 1);
      safePath[sizeof(safePath) - 1] = '\0';
    }

    if (!SD.exists(safePath)) return 0;

    OSSprite_t* spr = (OSSprite_t*)malloc(sizeof(OSSprite_t));
    if (!spr) return 0;

    File f = SD.open(safePath, FILE_READ);
    f.read((uint8_t*)tempSprite.frameBuffer(0), PACKED_BYTES_PER_SPRITE);
    f.close();

    spr->rawSize = compress4BitRLE((uint8_t*)tempSprite.frameBuffer(0), (uint8_t*)spr->rawSprite);
    spr->lastX = 0; spr->lastY = 0;
    spr->isOnScreen = false;

    return (uint32_t)spr;
  };

  kernelAPI.drawSprite = [](uint32_t spriteHandle, int x, int y) {
    OSSprite_t* spr = (OSSprite_t*)spriteHandle;
    if (!spr) return;

    if (spr->isOnScreen) {
      if (MATRIX_ACTIVE) {
        decompressRLEToCanvas(&tempSprite, 0, 0, (uint8_t*)spr->backupSprite, spr->backupSize);
        tempSprite.pushSprite(spr->lastX, spr->lastY);
      }
      spr->isOnScreen = false;
    }

    if (x < 0 || x > (TFT_WIDTH - SPRITE_SIZE) || y < 0 || y > (TFT_WIDTH - SPRITE_SIZE)) {
      return;
    }

    if (MATRIX_ACTIVE) {
      uint8_t* inb = (uint8_t*)bgCanvas.frameBuffer(0);
      uint8_t* out_backup = (uint8_t*)tempSprite.frameBuffer(0);
      uint8_t* out_render = (uint8_t*)renderSprite.frameBuffer(0);

      int bytesPerSpriteRow = SPRITE_SIZE / 2;
      int bytesPerCanvasRow = TFT_WIDTH / 2;

      int alignedX = x & ~1;
      int startByteX = alignedX >> 1;

      for (int row = 0; row < SPRITE_SIZE; row++) {
        int targetY = y + row;
        if (targetY >= TFT_WIDTH) break;

        int canvasSourceOffset = (targetY * bytesPerCanvasRow) + startByteX;
        int spriteDestOffset = row * bytesPerSpriteRow;

        memcpy(out_backup + spriteDestOffset, inb + canvasSourceOffset, bytesPerSpriteRow);
        memcpy(out_render + spriteDestOffset, inb + canvasSourceOffset, bytesPerSpriteRow);
      }
    }

    spr->backupSize = compress4BitRLE((uint8_t*)tempSprite.frameBuffer(0), (uint8_t*)spr->backupSprite);

    decompressRLEToCanvas(&tempSprite, 0, 0, (uint8_t*)spr->rawSprite, spr->rawSize);

    uint8_t* in = (uint8_t*)tempSprite.frameBuffer(0);
    uint8_t* out = (uint8_t*)renderSprite.frameBuffer(0);
    int totalPackedBytes = (SPRITE_SIZE * SPRITE_SIZE) / 2;

    for (int c = 0; c < totalPackedBytes; c++) {
      uint8_t inByte  = in[c];
      uint8_t outByte = out[c];

      uint8_t highIn = (inByte >> 4) & 0x0F;
      uint8_t lowIn  = inByte & 0x0F;

      uint8_t highOut = (outByte >> 4) & 0x0F;
      uint8_t lowOut  = outByte & 0x0F;

      if (highIn != 15) highOut = highIn;
      if (lowIn  != 15) lowOut  = lowIn;

      out[c] = (highOut << 4) | lowOut;
    }

    spr->lastX = x;
    spr->lastY = y;
    spr->isOnScreen = true;

    renderSprite.pushSprite(x, y);
  };

  kernelAPI.freeSprite = [](uint32_t spriteHandle) {
    OSSprite_t* spr = (OSSprite_t*)spriteHandle;
    if (spr) {
      if (spr->isOnScreen && MATRIX_ACTIVE) {
        decompressRLEToCanvas(&tempSprite, 0, 0, (uint8_t*)spr->backupSprite, spr->backupSize);
        tempSprite.pushSprite(spr->lastX, spr->lastY);
        spr->isOnScreen = false;
      }
      free(spr);
    }
  };

  kernelAPI.initGameMatrix = []() -> bool {
    if (!MATRIX_ACTIVE) {
      bgCanvas.setColorDepth(COLOR_DEPTH);
      bgCanvas.createSprite(TFT_WIDTH, TFT_WIDTH);
      bgCanvas.createPalette((uint16_t*)ramOSPalette);

      tempSprite.deleteSprite();
      tempSprite.setColorDepth(COLOR_DEPTH);
      tempSprite.createSprite(SPRITE_SIZE, SPRITE_SIZE);
      tempSprite.createPalette((uint16_t*)ramOSPalette);

      renderSprite.deleteSprite();
      renderSprite.setColorDepth(COLOR_DEPTH);
      renderSprite.createSprite(SPRITE_SIZE, SPRITE_SIZE);
      renderSprite.createPalette((uint16_t*)ramOSPalette);
    }
    bgCanvas.fillSprite(TFT_BLACK);
    bgCanvas.pushSprite(0, 0);
    return true;
  };

  kernelAPI.flushGameMatrix = []() {
    if (MATRIX_ACTIVE) {
      bgCanvas.pushSprite(0, 0);
    } else {
#if defined(BOARD_JC3248)
     canvas_bridge->flush();
#endif
    }
  };

  kernelAPI.closeGameMatrix = []() {
    renderSprite.deleteSprite();
    tempSprite.deleteSprite();
    bgCanvas.deleteSprite();
  };

}
