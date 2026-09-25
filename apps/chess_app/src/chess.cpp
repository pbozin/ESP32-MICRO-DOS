// src/chess.cpp
#include "microdos_api.h"
#include <cstring>

// Helper utility to convert column integers to characters safely bypassing memory padding
__attribute__((always_inline)) static inline char getColChar(int colIdx) {
    switch (colIdx) {
        case 0: return 'A'; case 1: return 'B'; case 2: return 'C'; case 3: return 'D';
        case 4: return 'E'; case 5: return 'F'; case 6: return 'G'; case 7: return 'H';
        default: return '\0';
    }
}

// Helper utility to convert row integers to characters safely bypassing memory padding
__attribute__((always_inline)) static inline char getRowChar(int rowIdx) {
    switch (rowIdx) {
        case 0: return '8'; case 1: return '7'; case 2: return '6'; case 3: return '5';
        case 4: return '4'; case 5: return '3'; case 6: return '2'; case 7: return '1';
        default: return '\0';
    }
}

static char textBuf[7] = "      "; // Initial label text (max 4 chars + null terminator)
int bufLen = 6;

#define ABS(x) ((x) < 0 ? -(x) : (x))
static bool humanIsWhite = true;
// Global piece sprite cache handle tracks
static int b[65]; 

__attribute__((always_inline)) static inline bool isMoveValid(int fromIdx, int toIdx) {
    if (fromIdx < 1 || fromIdx > 64 || toIdx < 1 || toIdx > 64) return false;
    if (fromIdx == toIdx) return false;

    int p = b[fromIdx];
    int t = b[toIdx];

    if (p == 0) return false;
    if (t != 0 && ((p > 0 && t > 0) || (p < 0 && t < 0))) return false;

    int fCol = (fromIdx - 1) % 8;
    int fRow = (fromIdx - 1) / 8;
    int tCol = (toIdx - 1) % 8;
    int tRow = (toIdx - 1) / 8;

    int dc = tCol - fCol;
    int dr = tRow - fRow;
    int absDc = ABS(dc);
    int absDr = ABS(dr);

    int sCol = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
    int sRow = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);

    int pieceType = ABS(p);

    // 1. PAWNS (IDs 1 to 8)
    if (pieceType >= 1 && pieceType <= 8) {
        bool movesUp = (p > 0) ? humanIsWhite : !humanIsWhite;

        if (movesUp) { // Moves Up (-Y direction)
            if (dc == 0 && dr == -1 && t == 0) return true;
            if (dc == 0 && dr == -2 && fRow == 6 && b[fromIdx - 8] == 0 && t == 0) return true;
            if (absDc == 1 && dr == -1 && t != 0) return true; // Captured opponent piece
        } else { // Moves Down (+Y direction)
            if (dc == 0 && dr == 1 && t == 0) return true;
            if (dc == 0 && dr == 2 && fRow == 1 && b[fromIdx + 8] == 0 && t == 0) return true;
            if (absDc == 1 && dr == 1 && t != 0) return true;
        }
        return false;
    }

    // 2. KNIGHTS (IDs 9 and 10)
    if (pieceType == 9 || pieceType == 10) {
        return (absDc == 1 && absDr == 2) || (absDc == 2 && absDr == 1);
    }

    // 3. BISHOPS (IDs 11 and 12)
    if (pieceType == 11 || pieceType == 12) {
        if (absDc != absDr) return false;
        int c = fCol + sCol, r = fRow + sRow;
        while (c != tCol && r != tRow) {
            // 🛠️ BOUNDS GUARD: Prevent creeping into illegal pointer lands
            if (c < 0 || c > 7 || r < 0 || r > 7) return false; 
            if (b[(r * 8) + c + 1] != 0) return false;
            c += sCol; r += sRow;
        }
        return true;
    }

    // 4. ROOKS (IDs 13 and 14)
    if (pieceType == 13 || pieceType == 14) {
        if (dc != 0 && dr != 0) return false;
        int c = fCol + sCol, r = fRow + sRow;
        while (c != tCol || r != tRow) {
            if (c < 0 || c > 7 || r < 0 || r > 7) return false;
            if (b[(r * 8) + c + 1] != 0) return false;
            c += sCol; r += sRow;
        }
        return true;
    }

    // 5. QUEENS (ID 15)
    if (pieceType == 15) {
        if (absDc != absDr && dc != 0 && dr != 0) return false;
        int c = fCol + sCol, r = fRow + sRow;
        while (c != tCol || r != tRow) {
            if (c < 0 || c > 7 || r < 0 || r > 7) return false;
            if (b[(r * 8) + c + 1] != 0) return false;
            c += sCol; r += sRow;
        }
        return true;
    }

    // 6. KINGS (ID 16)
    if (pieceType == 16) {
        return (absDc <= 1 && absDr <= 1);
    }

    return false;
}

__attribute__((always_inline)) static inline void drawBoard(MicroDosAPI* api) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int colorId = -1;
            colorId = ((r + c) % 2 == 0) ? 15 : 2;
            api->rect(c * 40, r * 40, 40, 40, colorId);
        }
    }
    api->flushGameMatrix();
}

static const int pieceWeights[] = {0, 100, 100, 100, 100, 100, 100, 100, 100, 300, 300, 300, 300, 500, 500, 900, 9999};

__attribute__((always_inline)) static inline void aiThinkAndRespond(MicroDosAPI* api) {
    int bestMoveFrom = 0;
    int bestMoveTo   = 0;
    
    // 🛠️ FIX 1: Dynamically assign worst-case initial scores based on AI piece color orientation
    int bestScore = humanIsWhite ? 99999 : -99999; 

    for (int from = 1; from <= 64; from++) {
        bool isAiPiece = humanIsWhite ? (b[from] < 0) : (b[from] > 0);

        if (isAiPiece) { 
            for (int to = 1; to <= 64; to++) {
                if (from == to) continue;
                
                if (b[to] != 0 && ((b[from] > 0 && b[to] > 0) || (b[from] < 0 && b[to] < 0))) continue;
                if (!isMoveValid(from, to)) continue;

                int capturedPiece = b[to];
                b[to] = b[from];
                b[from] = 0;

                int materialScore = 0;
                for (int i = 1; i <= 64; i++) {
                    int pieceVal = b[i];
                    if (pieceVal == 0) continue;
                    
                    // 🛠️ FIX 2: Prevent array overrun crashes by bounding the lookups strictly to 0-16 range
                    int pieceId = ABS(pieceVal);
                    if (pieceId > 16) pieceId = 16; 

                    if (pieceVal > 0) materialScore += pieceWeights[pieceId];
                    if (pieceVal < 0) materialScore -= pieceWeights[pieceId];
                }

                int toRow = (to - 1) / 8;
                int toCol = (to - 1) % 8;
                if (toRow >= 3 && toRow <= 4 && toCol >= 3 && toCol <= 4) {
                    // Encourage center control based on active side perspective rules
                    if (humanIsWhite) materialScore -= 10; 
                    else materialScore += 10;
                }

                // 🛠️ FIX 3: AI seeks lowest score if Black, but highest score if White!
                if (humanIsWhite) {
                    if (materialScore < bestScore) {
                        bestScore = materialScore;
                        bestMoveFrom = from;
                        bestMoveTo = to;
                    }
                } else {
                    if (materialScore > bestScore) {
                        bestScore = materialScore;
                        bestMoveFrom = from;
                        bestMoveTo = to;
                    }
                }

                b[from] = b[to];
                b[to] = capturedPiece;
            }
        }
    }

    if (bestMoveFrom != 0 && bestMoveTo != 0) {
        int fRow = (bestMoveFrom - 1) / 8;
        int fCol = (bestMoveFrom - 1) % 8;
        int tRow = (bestMoveTo - 1) / 8;
        int tCol = (bestMoveTo - 1) % 8;

        char cpuMove[6];
        cpuMove[0] = getColChar(fCol);
        cpuMove[1] = getRowChar(fRow);
        cpuMove[2] = getColChar(tCol);
        cpuMove[3] = getRowChar(tRow);
        cpuMove[4] = '\0';

        b[bestMoveTo] = b[bestMoveFrom];
        b[bestMoveFrom] = 0;
    }
}

__attribute__((always_inline)) static inline void initToledoBoard() {
    // 🛠️ Ensure absolute column parity between both setups so slider loops never miscalculate deltas
    int initialLayout[] = {13, 9, 11, 15, 16, 12, 10, 14}; 
    int initialLayoutF[] = {13, 9, 11, 16, 15, 12, 10, 14}; 
    
    for (int i = 0; i <= 64; i++) b[i] = 0;
    
    for (int c = 0; c < 8; c++) {
        if (humanIsWhite) {
            b[1 + c]  = -initialLayout[c]; // Row 0: Black AI
            b[9 + c]  = -(c + 1);          // Row 1: Black AI Pawns
            b[49 + c] = (c + 1);           // Row 6: White Human Pawns
            b[57 + c] = initialLayout[c];  // Row 7: White Human
        } else {
            b[1 + c]  = initialLayoutF[c];  // Row 0: White AI
            b[9 + c]  = (c + 1);           // Row 1: White AI Pawns
            b[49 + c] = -(c + 1);          // Row 6: Black Human Pawns
            b[57 + c] = -initialLayoutF[c]; // Row 7: Black Human
        }
    }
}

__attribute__((always_inline)) static inline void handlePawnPromotion() {
    // Pawns promote when they reach their own forward terminal row, 
    // regardless of whether the screen projection is inverted.
    
    // 1. Check Row 0 (Indices 1 to 8)
    for (int col = 0; col < 8; col++) {
        int idx = col + 1;
        if (b[idx] >= 1 && b[idx] <= 8)   b[idx] = 15;  // White Pawn becomes White Queen
        if (b[idx] <= -1 && b[idx] >= -8) b[idx] = -15; // Black Pawn becomes Black Queen
    }
    
    // 2. Check Row 7 (Indices 57 to 64)
    for (int col = 0; col < 8; col++) {
        int idx = 56 + col + 1;
        if (b[idx] >= 1 && b[idx] <= 8)   b[idx] = 15;
        if (b[idx] <= -1 && b[idx] >= -8) b[idx] = -15;
    }
}

__attribute__((always_inline)) static inline bool executeToledoMove(const char* moveStr) {
    if (!moveStr || moveStr[0] == '\0') return false;
    int fromCol = moveStr[0] - 'A';
    int fromRow = '8' - moveStr[1];
    int toCol   = moveStr[2] - 'A';
    int toRow   = '8' - moveStr[3];
    
    if (fromCol < 0 || fromCol > 7 || fromRow < 0 || fromRow > 7) return false;
    if (toCol < 0 || toCol > 7 || toRow < 0 || toRow > 7) return false;
    
    int fromIdx = (fromRow * 8) + fromCol + 1;
    int toIdx   = (toRow * 8) + toCol + 1;
    
    // 🛠️ INTEGRATION STEP: Filter constraints check
    if (!isMoveValid(fromIdx, toIdx)) return false;
    
    b[toIdx] = b[fromIdx];
    b[fromIdx] = 0;
    return true;
}

// Helper to locate a specific King square index (1-64)
static inline int findKing(bool whiteKing) {
    int targetId = whiteKing ? 16 : -16;
    for (int i = 1; i <= 64; i++) {
        if (b[i] == targetId) return i;
    }
    return 0; // Returns 0 if king is missing or captured in simulation
}

// Scans if a specific side's king is currently under line-of-sight attack
static inline bool isKingUnderAttack(bool isWhiteKing) {
    int kingIdx = findKing(isWhiteKing);
    // 🛠️ FIX 1: Hard guard rail exit. If kingIdx is 0, do not let it process coordinate math!
    if (kingIdx < 1 || kingIdx > 64) return false; 

    for (int attackerIdx = 1; attackerIdx <= 64; attackerIdx++) {
        int p = b[attackerIdx];
        if (p == 0) continue;

        if ((isWhiteKing && p < 0) || (!isWhiteKing && p > 0)) {
            if (isMoveValid(attackerIdx, kingIdx)) {
                return true; 
            }
        }
    }
    return false;
}

// Counts total legal moves remaining for a player. Returns 0 if locked down.
static inline int countLegalMoves(bool isWhiteTurn) {
    int legalCount = 0;

    for (int from = 1; from <= 64; from++) {
        int p = b[from];
        if (p == 0) continue;
        
        // 🛠️ FIX 2: Ensure correct turn assignment filters match your active color layout sets
        if ((isWhiteTurn && p < 0) || (!isWhiteTurn && p > 0)) continue;

        for (int to = 1; to <= 64; to++) {
            if (!isMoveValid(from, to)) continue;

            int backupTo = b[to];
            b[to] = b[from];
            b[from] = 0;

            bool selfCheck = isKingUnderAttack(isWhiteTurn);

            b[from] = b[to];
            b[to] = backupTo;

            if (!selfCheck) {
                legalCount++;
                return 1; // Early exit optimizing performance (1 is enough to prove game is alive)
            }
        }
    }
    return 0;
}

static uint32_t pieceSprites[33];

__attribute__((always_inline)) static inline void refreshBoard(MicroDosAPI* api) {
    
    handlePawnPromotion();

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int boardIdx = (r * 8) + c + 1;
            int pieceVal = b[boardIdx];

            if (pieceVal != 0) {
                  int spriteIdx = (pieceVal > 0) ? pieceVal : (-pieceVal + 16);
                  uint32_t handle = pieceSprites[spriteIdx];
                  if (handle != 0) {
                    api->drawSprite(handle, c * 40, r * 40);
		  }
            }
        }
    }
}

__attribute__((always_inline)) static inline void cacheAllChessSprites(MicroDosAPI* api) {
    pieceSprites[1]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[2]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[3]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[4]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[5]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[6]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[7]  = api->createSprite("Chess_plt60.spr");
    pieceSprites[8]  = api->createSprite("Chess_plt60.spr");

    pieceSprites[9]  = api->createSprite("Chess_nlt60.spr");
    pieceSprites[10] = api->createSprite("Chess_nlt60.spr");
    pieceSprites[11] = api->createSprite("Chess_blt60.spr"); 
    pieceSprites[12] = api->createSprite("Chess_blt60.spr"); 
    pieceSprites[13] = api->createSprite("Chess_rlt60.spr");
    pieceSprites[14] = api->createSprite("Chess_rlt60.spr");

    pieceSprites[15] = api->createSprite("Chess_qlt60.spr");
    pieceSprites[16] = api->createSprite("Chess_klt60.spr");

    pieceSprites[17] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[18] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[19] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[20] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[21] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[22] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[23] = api->createSprite("Chess_pdt60.spr");
    pieceSprites[24] = api->createSprite("Chess_pdt60.spr");

    pieceSprites[25] = api->createSprite("Chess_ndt60.spr");
    pieceSprites[26] = api->createSprite("Chess_ndt60.spr");
    pieceSprites[27] = api->createSprite("Chess_bdt60.spr");
    pieceSprites[28] = api->createSprite("Chess_bdt60.spr");
    pieceSprites[29] = api->createSprite("Chess_rdt60.spr");
    pieceSprites[30] = api->createSprite("Chess_rdt60.spr");

    pieceSprites[31] = api->createSprite("Chess_qdt60.spr");
    pieceSprites[32] = api->createSprite("Chess_kdt60.spr");
}

#define UNDO_DEPTH 6
static int8_t undoHistory[UNDO_DEPTH][65];
static int undoHead = 0;
static int undoCount = 0;

__attribute__((always_inline)) static inline void saveUndoState() {
    // Copy the current board layout to our cyclic history ring buffer
    for(int i = 0; i <= 64; i++) {
        undoHistory[undoHead][i] = b[i];
    }
    undoHead = (undoHead + 1) % UNDO_DEPTH;
    if (undoCount < UNDO_DEPTH) undoCount++;
}

__attribute__((always_inline)) static inline bool executeUndo() {
    if (undoCount <= 0) return false;
    
    // Step backwards through the cyclic pointer ring
    undoHead = (undoHead - 1 + UNDO_DEPTH) % UNDO_DEPTH;
    for(int i = 0; i <= 64; i++) {
        b[i] = undoHistory[undoHead][i];
    }
    undoCount--;
    return true;
}
__attribute__((always_inline)) static inline void clearUndoHistory() {
    undoHead = 0;
    undoCount = 0;
}


#define TKN_F5 '\x15'


extern "C" __attribute__((section(".text._start"))) int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api; 
    if (!api) return -1;
    if (!api->initGameMatrix()) return -1;

    api->clear();
    api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
    cacheAllChessSprites(api);
    initToledoBoard(); 
    drawBoard(api);
    refreshBoard(api); 

    int selectX = -1, selectY = -1;
    bool running = true;
    bool playerTurn = true;
    char moveStr[5] = ""; 

    while (running) {
        int key = api->inkey();
        if (key == TKN_F5 || key == 'Q' || key == 'q') { 
            running = false;
            break;
        }
        if (key == '\x12' || key == 'U' || key == 'u') {
            if (executeUndo()) {
                selectX = -1; selectY = -1;
                refreshBoard(api);
                api->delay(300);
            }
            continue;
        }
        if (key == '\x11') { 
            humanIsWhite = true;
            playerTurn = true; // White always moves first
            selectX = -1; selectY = -1;
            initToledoBoard();
	    clearUndoHistory();
	    strcpy(textBuf, "      ");
	    bufLen = 6;
            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
            drawBoard(api);
            for (int i = 1; i <= 32; i++) {
              if (pieceSprites[i] != 0) {
	        *(bool*)pieceSprites[i] = false;
              }
            }
            refreshBoard(api); 
	    api->delay(300);
	    continue;
        }
        if (key == '\x13') { // F3 / "FLIP"
            humanIsWhite = !humanIsWhite;
            selectX = -1; selectY = -1;
            // Clear host tracking parameters to prevent overlay corruption
            for (int i = 1; i <= 32; i++) {
                if (pieceSprites[i] != 0) *(bool*)pieceSprites[i] = false;
            }

            initToledoBoard();
            clearUndoHistory();
            playerTurn = true; // White always moves first
            drawBoard(api);
            refreshBoard(api);

	    strcpy(textBuf, "      ");
	    bufLen = 6;
            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));

            // 🛠️ PERTURBATION: If human is Black, AI must make the first move instantly!
            if (!humanIsWhite) {
                saveUndoState(); // Capture state layout before AI response moves pieces
                playerTurn = false;
                aiThinkAndRespond(api); // AI executes White's opening move
                refreshBoard(api);
                playerTurn = true; // Hand turn back to human (now playing Black)
            }
            
            api->delay(300);
            continue;
        }
        // --- Handle Enter Key Pass ---
        if (key == '\t') {
            if (bufLen > 0) {
                textBuf[--bufLen] = '\0';
                if (bufLen == 0) {
                    strcpy(textBuf, "      ");
                    bufLen = 6;
                }
                api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), textBuf, STRING(" QUIT "));
            }
	    api->delay(200);
	    continue;
        }
        // --- Handle Backspace Key Pass ---
        if (key == '\n') {
            if (bufLen == 4) {
                moveStr[0] = textBuf[0];
                moveStr[1] = textBuf[1];
                moveStr[2] = textBuf[2];
                moveStr[3] = textBuf[3];
                moveStr[4] = '\0';

                saveUndoState();
                if (executeToledoMove(moveStr)) {
                    refreshBoard(api); 
                    api->delay(200);
                    
                    playerTurn = false;
                    aiThinkAndRespond(api);
                    refreshBoard(api); 
                    api->delay(200);
                    
                    playerTurn = true;

                    if (countLegalMoves(humanIsWhite) == 0 || countLegalMoves(!humanIsWhite) == 0) { 
                        if (isKingUnderAttack(humanIsWhite) || isKingUnderAttack(!humanIsWhite)) {
                            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("MATE!!"), STRING(" QUIT "));
			} else {
                            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("DRAW!!"), STRING(" QUIT "));
			}
                        bufLen = 6;
                        selectX = -1;
	       	        selectY = -1;
		        api->delay(2000);
	                continue;
                    }
                } else {
                    undoHead = (undoHead - 1 + UNDO_DEPTH) % UNDO_DEPTH;
                    if (undoCount > 0) undoCount--;
                    refreshBoard(api); 
                }
		strcpy(textBuf, "      ");
                bufLen = 6;
                selectX = -1;
	       	selectY = -1;
                api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
            }
	    api->delay(300);
	    continue;
        }
        // --- Handle Standard Printable Characters (A-Z, 0-9, etc.) ---
        else if (key >= 32 && key <= 126) {
            if (bufLen == 6 && strcmp(textBuf, "      ") == 0) {
                bufLen = 0;
                memset(textBuf, 0, sizeof(textBuf));
            }

            // Append if there is remaining space in the 4-character buffer layout limit
            if (bufLen < 4) {
                textBuf[bufLen++] = (char)key;
                textBuf[bufLen] = '\0';
                api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), textBuf, STRING(" QUIT "));
            }
	    api->delay(200);
	    continue;
        }

        TouchState touch;
        api->getTouch(&touch);
        
        if (touch.isPressed && touch.y >= 0 && touch.y < 320 && touch.x >= 0 && touch.x < 320) {
            int gridX = touch.x / 40;
            int gridY = touch.y / 40;

            if (selectX == -1) {
                int selectIdx = (gridY * 8) + gridX + 1;
                int piece = b[selectIdx];

                bool isHumanPiece = humanIsWhite ? (piece > 0) : (piece < 0);

                if (piece != 0 && playerTurn && isHumanPiece) { 
                    selectX = gridX;
                    selectY = gridY;
                    //api->rect(selectX * 40, selectY * 40, 40, 40, 3); 
                }
            } else {
                moveStr[0] = getColChar(selectX);
                moveStr[1] = getRowChar(selectY);
                moveStr[2] = getColChar(gridX);
                moveStr[3] = getRowChar(gridY);
                moveStr[4] = '\0';

                saveUndoState(); 
                if (executeToledoMove(moveStr)) {
                    refreshBoard(api); 
                    api->delay(200);
                    
                    playerTurn = false;
                    aiThinkAndRespond(api);
                    refreshBoard(api); 
                    api->delay(200);
                    
                    playerTurn = true;

                    if (countLegalMoves(humanIsWhite) == 0 || countLegalMoves(!humanIsWhite) == 0) { 
                        if (isKingUnderAttack(humanIsWhite) || isKingUnderAttack(!humanIsWhite)) {
                            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("MATE!!"), STRING(" QUIT "));
			} else {
                            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("DRAW!!"), STRING(" QUIT "));
			}
		        strcpy(textBuf, "      ");
                        bufLen = 6;
                        selectX = -1;
	       	        selectY = -1;
			api->delay(2000);
			continue;
                    }
                } else {
                    undoHead = (undoHead - 1 + UNDO_DEPTH) % UNDO_DEPTH;
                    if (undoCount > 0) undoCount--;
                    refreshBoard(api); 
                }
		strcpy(textBuf, "      ");
		bufLen = 6;
                selectX = -1;
                selectY = -1;
                api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
            }
            api->delay(300); 
        }
        api->delay(30);
    }

    api->clearFKeys();
    for (int i = 1; i <= 32; i++) {
        if (pieceSprites[i] != 0) api->freeSprite(pieceSprites[i]);
    }
    api->closeGameMatrix();
    api->delay(100);
    return 0;
}
