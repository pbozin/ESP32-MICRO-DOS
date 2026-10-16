#define BOARD_SIZE 9
#define COLOR_BACKGROUND 2
#define COLOR_GRID      15 
#define COLOR_BLACK      0
#define COLOR_WHITE      1
#define COLOR_TEXT       6 

enum Stone { EMPTY = 0, BLACK = 1, WHITE = 2 };

int previousBoard[BOARD_SIZE][BOARD_SIZE];
int previousPlayer = BLACK;
int hasUndoHistory = 0;

int blackCaptures = 0;
int whiteCaptures = 0;
char textBuf[16] = "B:0 W:0";

int board[BOARD_SIZE][BOARD_SIZE];
int currentPlayer = BLACK;
int cellWidth, cellHeight;
int boardOffsetX, boardOffsetY;
int boardWidth, boardHeight;

bool visited[BOARD_SIZE][BOARD_SIZE];

void saveUndoState() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            previousBoard[y][x] = board[y][x];
        }
    }
    previousPlayer = currentPlayer;
    hasUndoHistory = 1;
}

void performUndo() {
    if (!hasUndoHistory) return;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = previousBoard[y][x];
        }
    }
    currentPlayer = previousPlayer;
    hasUndoHistory = 0;
}

void updateStatusText() {
    int len = 0;
    textBuf[0] = (currentPlayer == BLACK) ? '*' : ' ';
    textBuf[1] = 'B'; textBuf[2] = ':'; textBuf[3] = '0' + (blackCaptures % 10);
    textBuf[4] = ' ';
    textBuf[5] = (currentPlayer == WHITE) ? '*' : ' ';
    textBuf[6] = 'W'; textBuf[7] = ':'; textBuf[8] = '0' + (whiteCaptures % 10);
    textBuf[9] = '\0';
}

void initGame() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = EMPTY;
        }
    }
    currentPlayer = BLACK;

    int screenW = getTermWidth();
    int screenH = getTermHeight();
    
    int minDim = (screenW < screenH) ? screenW : screenH;
    boardWidth = minDim - 20; 
    boardHeight = boardWidth;
    
    boardOffsetX = (screenW - boardWidth) / 2;
    boardOffsetY = (screenH - boardHeight) / 2;
    
    cellWidth = boardWidth / (BOARD_SIZE - 1);
    cellHeight = boardHeight / (BOARD_SIZE - 1);
}

void renderGame() {
    fillRect(boardOffsetX - 10, boardOffsetY - 10, boardWidth + 20, boardHeight + 20, COLOR_BACKGROUND);
    
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
    
    int radius = (cellWidth < cellHeight ? cellWidth : cellHeight) * 45 / 100;
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int px = boardOffsetX + (x * cellWidth);
            int py = boardOffsetY + (y * cellHeight);
            
            if (board[y][x] == BLACK) {
                fillCircle(px, py, radius, COLOR_BLACK);
            } else if (board[y][x] == WHITE) {
                fillCircle(px, py, radius, COLOR_WHITE);
                drawCircle(px, py, radius, COLOR_GRID);
            }
        }
    }
    
    flushGameMatrix();
}

void countLiberties(int x, int y, int stoneType, int &liberties, int &groupSize, int groupX[81], int groupY[81]) {
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
            if (board[ny][nx] == EMPTY) {
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

bool processMove(int targetX, int targetY, int player) {
    if (board[targetY][targetX] != EMPTY) return false;

    int tempBoard[BOARD_SIZE][BOARD_SIZE];
    for(int r=0; r<BOARD_SIZE; r++) {
        for(int c=0; c<BOARD_SIZE; c++) tempBoard[r][c] = board[r][c];
    }

    board[targetY][targetX] = player;
    
    int opponent = (player == BLACK) ? WHITE : BLACK;
    bool captureHappened = false;
    int turnCaptures = 0;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    
    for (int i = 0; i < 4; i++) {
        int nx = targetX + dx[i];
        int ny = targetY + dy[i];
        
        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE && board[ny][nx] == opponent) {
            for(int r=0; r<BOARD_SIZE; r++) for(int c=0; c<BOARD_SIZE; c++) visited[r][c] = false;
            
            int liberties = 0;
            int groupSize = 0;
            int groupX[BOARD_SIZE * BOARD_SIZE];
            int groupY[BOARD_SIZE * BOARD_SIZE];
            
            countLiberties(nx, ny, opponent, liberties, groupSize, groupX, groupY);
            
            if (liberties == 0) {
                for (int g = 0; g < groupSize; g++) {
                    board[groupY[g]][groupX[g]] = EMPTY;
                    turnCaptures++;
                }
                captureHappened = true;
            }
        }
    }

    if (!captureHappened) {
        for(int r=0; r<BOARD_SIZE; r++) for(int c=0; c<BOARD_SIZE; c++) visited[r][c] = false;
        int liberties = 0;
        int groupSize = 0;
        int groupX[BOARD_SIZE * BOARD_SIZE];
        int groupY[BOARD_SIZE * BOARD_SIZE];
        
        countLiberties(targetX, targetY, player, liberties, groupSize, groupX, groupY);
        if (liberties == 0) {
            board[targetY][targetX] = EMPTY;
            return false;
        }
    }

    for(int r=0; r<BOARD_SIZE; r++) {
        for(int c=0; c<BOARD_SIZE; c++) previousBoard[r][c] = tempBoard[r][c];
    }
    previousPlayer = player;
    hasUndoHistory = 1;

    if (player == BLACK) blackCaptures += turnCaptures;
    else whiteCaptures += turnCaptures;

    return true; 
}

void main() {
    initGame();
    initGameMatrix();
    
    blackCaptures = 0;
    whiteCaptures = 0;
    hasUndoHistory = 0;
    
    updateStatusText();
    setFKeys(" NEW  ", " UNDO ", " PASS ", textBuf, " QUIT ");
    
    int wasPressedLastFrame = 0;
    bool running = true;
    
    while (running) {
        renderGame();
        
        int key = getchar();
        
        if (key != -1) { 
            if (key == '\x15' || key == 'Q' || key == 'q') {
                running = false;
                break;
            }
            
            if (key == '\x12' || key == 'U' || key == 'u') {
                performUndo();
                updateStatusText();
                setFKeys(" NEW  ", " UNDO ", " PASS ", textBuf, " QUIT ");
                delay(300);
                continue;
            }
            
            if (key == '\x11' || key == 'N' || key == 'n') {
                initGame();
                blackCaptures = 0;
                whiteCaptures = 0;
                hasUndoHistory = 0;
                updateStatusText();
                setFKeys(" NEW  ", " UNDO ", " PASS ", textBuf, " QUIT ");
                delay(300);
                continue;
            }
            
            if (key == '\x13' || key == 'P' || key == 'p') {
                saveUndoState();
                currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
                updateStatusText();
                setFKeys(" NEW  ", " UNDO ", " PASS ", textBuf, " QUIT ");
                delay(300);
                continue;
            }
        }
        
        struct TouchState touch;
        getTouch(&touch);
        
        if (touch.isPressed && !wasPressedLastFrame) { 
            if (touch.x >= (boardOffsetX - cellWidth / 2) && 
                touch.x <= (boardOffsetX + boardWidth + cellWidth / 2) &&
                touch.y >= (boardOffsetY - cellHeight / 2) && 
                touch.y <= (boardOffsetY + boardHeight + cellHeight / 2)) {
                
                int gridX = (touch.x - boardOffsetX + (cellWidth / 2)) / cellWidth;
                int gridY = (touch.y - boardOffsetY + (cellHeight / 2)) / cellHeight;
                
                if (gridX >= 0 && gridX < BOARD_SIZE && gridY >= 0 && gridY < BOARD_SIZE) {
                    if (processMove(gridX, gridY, currentPlayer)) {
                        currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
                        updateStatusText();
                        setFKeys(" NEW  ", " UNDO ", " PASS ", textBuf, " QUIT ");
                    }
                }
            }
        }
        
        wasPressedLastFrame = touch.isPressed;
        delay(16);
    }
    
    clearFKeys();
    closeGameMatrix();
}

