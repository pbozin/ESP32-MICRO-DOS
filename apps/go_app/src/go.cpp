#include <microdos_api.h>
#include <microdos_util.h>

#define BOARD_SIZE 9
#define COLOR_BACKGROUND 2
#define COLOR_GRID      15
#define COLOR_BLACK      0
#define COLOR_WHITE      1

NEW_STRING(system_prompt, "You are a Weiqi (GO game) engine bot. "
		          "You are playing White (O) on a 9x9 board. "
			  "Black is (X). "
			  "Empty field is (.). "
			  "You can place a stone only on empty field (.). "
			  "Reply with EXACTLY ONE coordinate consisting of a letter (column) and a number (row), eg. (A1, I9, G4) for your best legal move. "
			  "Do not write anything else.");
enum Stone {S_EMPTY = 0, S_BLACK = 1, S_WHITE = 2, NOTHING = 0 };

static int previousBoard[BOARD_SIZE][BOARD_SIZE] ALIGNED;
static int previousPlayer = S_BLACK;
static int hasUndoHistory = 0;

static int blackCaptures = 0;
static int whiteCaptures = 0;
static char textBuf[16] ALIGNED = "B:0 W:0";

static int board[BOARD_SIZE][BOARD_SIZE] ALIGNED;
static int currentPlayer = S_BLACK;
static int cellWidth, cellHeight;
static int boardOffsetX, boardOffsetY;
static int boardWidth, boardHeight;

static bool visited[BOARD_SIZE][BOARD_SIZE] ALIGNED;

ALWAYS INLINE void saveUndoState() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            previousBoard[y][x] = board[y][x];
        }
    }
    previousPlayer = currentPlayer;
    hasUndoHistory = 1;
}

ALWAYS INLINE void performUndo() {
    if (!hasUndoHistory) return;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = previousBoard[y][x];
        }
    }
    currentPlayer = previousPlayer;
    hasUndoHistory = 0;
}

ALWAYS INLINE void updateStatusText() {
    textBuf[0] = (currentPlayer == S_BLACK) ? '*' : ' ';
    textBuf[1] = 'B'; textBuf[2] = ':'; textBuf[3] = '0' + (blackCaptures % 10);
    textBuf[4] = ' ';
    textBuf[5] = (currentPlayer == S_WHITE) ? '*' : ' ';
    textBuf[6] = 'W'; textBuf[7] = ':'; textBuf[8] = '0' + (whiteCaptures % 10);
    textBuf[9] = '\0';

    setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" PASS "), textBuf, STRING(" QUIT "));
}

ALWAYS INLINE void initGame() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = S_EMPTY;
        }
    }
    currentPlayer = S_BLACK;

    int screenW = getTermWidth();
    int screenH = getTermHeight();

    int minDim = (screenW < screenH) ? screenW : screenH;
    boardWidth = minDim - 30;
    boardHeight = boardWidth;

    boardOffsetX = (screenW - boardWidth) / 2;
    boardOffsetY = (screenH - boardHeight) / 2;

    cellWidth = boardWidth / (BOARD_SIZE - 1);
    cellHeight = boardHeight / (BOARD_SIZE - 1);
}

ALWAYS INLINE void renderGame() {
    fillRect(boardOffsetX - 15, boardOffsetY - 15, boardWidth + 30, boardHeight + 30, COLOR_BACKGROUND);

    for (int i = 0; i < BOARD_SIZE; i++) {
        int x = boardOffsetX + (i * cellWidth);
        int y = boardOffsetY + (i * cellHeight);

        drawLine(x, boardOffsetY, x, boardOffsetY + boardHeight, COLOR_GRID);
        drawLine(boardOffsetX, y, boardOffsetX + boardWidth, y, COLOR_GRID);
    }

    int starPoints[] = {2, 4, 6};
    for(int sy = 0; sy < 3; sy++) {
        for(int sx = 0; sx < 3; sx++) {
            int px = boardOffsetX + (starPoints[sx] * cellWidth);
            int py = boardOffsetY + (starPoints[sy] * cellHeight);
            fillCircle(px, py, 3, COLOR_GRID);
        }
    }

    int radius = (cellWidth < cellHeight ? cellWidth : cellHeight) * 45 / 135;

    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int px = boardOffsetX + (x * cellWidth);
            int py = boardOffsetY + (y * cellHeight);

            if (board[y][x] == S_BLACK) {
                fillCircle(px, py, radius, COLOR_BLACK);
            } else if (board[y][x] == S_WHITE) {
                fillCircle(px, py, radius, COLOR_WHITE);
                drawCircle(px, py, radius, COLOR_GRID);
            }
        }
    }

    flushGameMatrix();
}

INLINE void countLiberties(int x, int y, int stoneType, int &liberties, int &groupSize, int groupX[], int groupY[]) {
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) return;
    if (visited[y][x]) return;
    if (board[y][x] != stoneType) return;

    visited[y][x] = true;
    groupX[groupSize] = x;
    groupY[groupSize] = y;
    groupSize++;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
            if (board[ny][nx] == S_EMPTY) {
                if (!visited[ny][nx]) {
                    liberties++;
                    visited[ny][nx] = true;
                }
            } else if (board[ny][nx] == stoneType && !visited[ny][nx]) {
                countLiberties(nx, ny, stoneType, liberties, groupSize, groupX, groupY);
            }
        }
    }
}

ALWAYS INLINE bool processMove(int targetX, int targetY, int player) {
    if (board[targetY][targetX] != S_EMPTY) return false;

    int tempBoard[BOARD_SIZE][BOARD_SIZE];
    for(int r = 0; r < BOARD_SIZE; r++) {
        for(int c = 0; c < BOARD_SIZE; c++) tempBoard[r][c] = board[r][c];
    }

    board[targetY][targetX] = player;

    int opponent = (player == S_BLACK) ? S_WHITE : S_BLACK;
    bool captureHappened = false;
    int turnCaptures = 0;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};

    for (int i = 0; i < 4; i++) {
        int nx = targetX + dx[i];
        int ny = targetY + dy[i];

        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[ny][nx] == opponent) {
            for(int r = 0; r < BOARD_SIZE; r++) for(int c = 0; c < BOARD_SIZE; c++) visited[r][c] = false;

            int liberties = 0;
            int groupSize = 0;
            int groupX[BOARD_SIZE * BOARD_SIZE];
            int groupY[BOARD_SIZE * BOARD_SIZE];

            countLiberties(nx, ny, opponent, liberties, groupSize, groupX, groupY);

            if (liberties == 0) {
                for (int g = 0; g < groupSize; g++) {
                    board[groupY[g]][groupX[g]] = S_EMPTY;
                    turnCaptures++;
                }
                captureHappened = true;
            }
        }
    }

    if (!captureHappened) {
        for(int r = 0; r < BOARD_SIZE; r++) for(int c = 0; c < BOARD_SIZE; c++) visited[r][c] = false;
        int liberties = 0;
        int groupSize = 0;
        int groupX[BOARD_SIZE * BOARD_SIZE];
        int groupY[BOARD_SIZE * BOARD_SIZE];

        countLiberties(targetX, targetY, player, liberties, groupSize, groupX, groupY);
        if (liberties == 0) {
            board[targetY][targetX] = S_EMPTY;
            return false;
        }
    }

    for(int r = 0; r < BOARD_SIZE; r++) {
        for(int c = 0; c < BOARD_SIZE; c++) previousBoard[r][c] = tempBoard[r][c];
    }
    previousPlayer = player;
    hasUndoHistory = 1;

    if (player == S_BLACK) blackCaptures += turnCaptures;
    else whiteCaptures += turnCaptures;

    return true;
}

ALWAYS INLINE void serializeBoard(char *outputPrompt) {
    strcpy(outputPrompt, STRING("-------------------\nCurrent Board State:\n"));
    strcat(outputPrompt, STRING("  A B C D E F G H I\n"));

    for (int y = 0; y < BOARD_SIZE; y++) {
        char lineBuf[32];
	itoa(y+1, lineBuf);
        strcat(outputPrompt, lineBuf);

        for (int x = 0; x < BOARD_SIZE; x++) {
            if (board[y][x] == S_BLACK) strcat(outputPrompt, STRING(" X"));
            else if (board[y][x] == S_WHITE) strcat(outputPrompt, STRING(" O"));
            else strcat(outputPrompt, STRING(" ."));
        }
        strcat(outputPrompt, STRING("\n"));
    }
    strcat(outputPrompt, STRING("-------------------\n"));
}

ALWAYS INLINE bool getAiMoveFromOllama(int &outX, int &outY) {
    char userPrompt[512] ALIGNED;
    char responseBuf[64] ALIGNED;

    serializeBoard(userPrompt);

    int success = ollamaStream(userPrompt, STRING("CFG"), STRING("CFG"), system_prompt, responseBuf, sizeof(responseBuf), 0);

    if (success < 0) return false;

    char colChar = ' ';
    int rowNum = -1;

    for (int i = 0; responseBuf[i] != '\0' && i < 60; i++) {
        char c = responseBuf[i];
        if ((c >= 'A' && c <= 'J' && c != 'I') || (c >= 'a' && c <= 'j' && c != 'i')) {
            if (responseBuf[i+1] >= '1' && responseBuf[i+1] <= '9') {
                colChar = c;
                rowNum = responseBuf[i+1] - '0';
                break;
            }
        }
    }

    if (rowNum == -1) return false;

    if (colChar >= 'a') colChar -= 32;

    if (colChar >= 'J') outX = colChar - 'A' - 1;
    else outX = colChar - 'A';

    outY = rowNum - 1;

    return (outX >= 0 && outX < BOARD_SIZE && outY >= 0 && outY < BOARD_SIZE);
}


int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;

    blackCaptures = 0;
    whiteCaptures = 0;
    hasUndoHistory = 0;

    int wasPressedLastFrame = 0;

    termPrintln(STRING("CONNECTING..."));

    if (wifiUp(STRING("CFG"), STRING("CFG")) != 0) {
        termPrintln(STRING("ERR: WI-FI PROVISIONING FAILED."));
        return -1;
    }
    delay(500);

    int success = ollamaStream(STRING("NEW"), STRING("CFG"), STRING("CFG"), system_prompt, NULL, 0, 0);
    if (success < 0) {
        termPrintln(STRING("ERR: OLLAMA CONNECTION FAILED."));
        return -1;
    }

    clearScreen();

    if (!initGameMatrix()) {
	wifiDown();
        termPrintln(STRING("ERR: GAME INIT FAILED."));
        return -1;
    }

    initGame();
    updateStatusText();
    bool running = true;

    while (running) {
        renderGame();

        if (currentPlayer == S_WHITE) {
            setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" THNKNG"), textBuf, STRING(" QUIT "));
            renderGame();

            int aiX, aiY;
            bool aiMoved = false;

            for (int attempts = 0; attempts < 3; attempts++) {
                if (getAiMoveFromOllama(aiX, aiY)) {
                    if (processMove(aiX, aiY, S_WHITE)) {
                        aiMoved = true;
                        break;
                    }
                }
            }

            if (!aiMoved) {
                saveUndoState();
            }

            currentPlayer = S_BLACK;
            updateStatusText();
            continue;
        }

        int key = getKey();
        if (key != -1) {
            if (key == '\x15' || key == 'Q' || key == 'q') {
		    running = false;
		    break; }
            if (key == '\x12' || key == 'U' || key == 'u') {
		    performUndo();
		    updateStatusText();
		    continue;
	    }
            if (key == '\x11' || key == 'N' || key == 'n') {
                    ollamaStream(STRING("NEW"), STRING("CFG"), STRING("CFG"), system_prompt, NULL, 0, 0);
		    initGame();
		    blackCaptures = 0;
		    whiteCaptures = 0;
		    updateStatusText();
		    continue;
	    }
            if (key == '\x13' || key == 'P' || key == 'p') {
		    saveUndoState();
		    currentPlayer = S_WHITE;
		    updateStatusText();
		    continue;
	    }
        }

        TouchState touch;
        getTouch(&touch);

        if (touch.isPressed && !wasPressedLastFrame) {
            if (touch.x >= (boardOffsetX - cellWidth / 2) && touch.x <= (boardOffsetX + boardWidth + cellWidth / 2) &&
                touch.y >= (boardOffsetY - cellHeight / 2) && touch.y <= (boardOffsetY + boardHeight + cellHeight / 2)) {

                int gridX = (touch.x - boardOffsetX + (cellWidth / 2)) / cellWidth;
                int gridY = (touch.y - boardOffsetY + (cellHeight / 2)) / cellHeight;

                if (gridX >= 0 && gridX < BOARD_SIZE && gridY >= 0 && gridY < BOARD_SIZE) {
                    if (processMove(gridX, gridY, S_BLACK)) {
                        currentPlayer = S_WHITE;
                        updateStatusText();
                    }
                }
            }
        }

        wasPressedLastFrame = touch.isPressed;
        delay(16);
    }
    wifiDown();
    clearFKeys();
    closeGameMatrix();
    return 0;
}

