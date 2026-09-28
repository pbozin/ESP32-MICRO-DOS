#include "microdos_api.h"
#include <cstring>

static const int pieceWeights[] = {0, 100, 100, 100, 100, 100, 100, 100, 100, 300, 300, 300, 300, 500, 500, 900, 9999};

static uint32_t pieceSprites[33];

#define UNDO_DEPTH 6
static int8_t undoHistory[UNDO_DEPTH][65];
static int undoHead = 0;
static int undoCount = 0;

static char textBuf[7] = "      ";
int bufLen = 6;

#define ABS(x) ((x) < 0 ? -(x) : (x))
static bool humanIsWhite = true;

static int b[65];

__attribute__((always_inline)) static inline bool isMoveValid(int fromIdx, int toIdx);

static inline int findKing(bool whiteKing) {
    int targetId = whiteKing ? 16 : -16;
    for (int i = 1; i <= 64; i++) {
        if (b[i] == targetId) return i;
    }
    return 0;
}

static inline bool isKingUnderAttack(bool isWhiteKing) {
    int kingIdx = findKing(isWhiteKing);
    if (kingIdx < 1 || kingIdx > 64) return false;

    for (int attackerIdx = 1; attackerIdx <= 64; attackerIdx++) {
        int p = b[attackerIdx];
        if (p == 0) continue;

        if ((isWhiteKing && p < 0) || (!isWhiteKing && p > 0)) {
            if (attackerIdx != kingIdx && isMoveValid(attackerIdx, kingIdx)) {
                return true;
            }
        }
    }
    return false;
}

static inline int countLegalMoves(bool isWhiteTurn) {
    int legalCount = 0;

    for (int from = 1; from <= 64; from++) {
        int p = b[from];
        if (p == 0) continue;

        // Skip pieces that do not match the current turn's color
        if (isWhiteTurn && p < 0) continue;  // White turn: Skip black pieces
        if (!isWhiteTurn && p > 0) continue; // Black turn: Skip white pieces

        for (int to = 1; to <= 64; to++) {
            if (from == to) continue;

            // Prevent capturing pieces of your own color
            if (b[to] != 0) {
                if (isWhiteTurn && b[to] > 0) continue;
                if (!isWhiteTurn && b[to] < 0) continue;
            }

            // Verify basic geometric legality
            if (!isMoveValid(from, to)) continue;

            // Simulate move
            int backupTo = b[to];
            b[to] = b[from];
            b[from] = 0;

            // Ensure this move doesn't leave/put our own king in check
            bool selfCheck = isKingUnderAttack(isWhiteTurn);

            // Rollback simulation
            b[from] = b[to];
            b[to] = backupTo;

            if (!selfCheck) {
                legalCount++;
            }
        }
    }
    return legalCount;
}

static inline bool executeToledoMove(const char* moveStr) {
    if (!moveStr || moveStr[0] == '\0') return false;
    int fromCol = moveStr[0] - 'A';
    int fromRow = '8' - moveStr[1];
    int toCol   = moveStr[2] - 'A';
    int toRow   = '8' - moveStr[3];

    if (fromCol < 0 || fromCol > 7 || fromRow < 0 || fromRow > 7) return false;
    if (toCol < 0 || toCol > 7 || toRow < 0 || toRow > 7) return false;

    int fromIdx = (fromRow * 8) + fromCol + 1;
    int toIdx   = (toRow * 8) + toCol + 1;

    if (!isMoveValid(fromIdx, toIdx)) return false;

    bool isWhiteMove = (b[fromIdx] > 0);

    // Simulate move to catch self-check violations
    int capturedPiece = b[toIdx];
    b[toIdx] = b[fromIdx];
    b[fromIdx] = 0;

    bool leavesKingInCheck = isKingUnderAttack(isWhiteMove);

    // Roll back simulation
    b[fromIdx] = b[toIdx];
    b[toIdx] = capturedPiece;

    if (leavesKingInCheck) return false;

    // Commit the valid move
    b[toIdx] = b[fromIdx];
    b[fromIdx] = 0;
    return true;
}

static inline void refreshBoard(MicroDosAPI* api) {
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

static inline void cacheAllChessSprites(MicroDosAPI* api) {
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

static inline void saveUndoState() {
    for(int i = 0; i <= 64; i++) {
        undoHistory[undoHead][i] = b[i];
    }
    undoHead = (undoHead + 1) % UNDO_DEPTH;
    if (undoCount < UNDO_DEPTH) undoCount++;
}

static inline bool executeUndo() {
    if (undoCount <= 0) return false;

    undoHead = (undoHead - 1 + UNDO_DEPTH) % UNDO_DEPTH;
    for(int i = 0; i <= 64; i++) {
        b[i] = undoHistory[undoHead][i];
    }
    undoCount--;
    return true;
}

static inline void clearUndoHistory() {
    undoHead = 0;
    undoCount = 0;
}

static inline void drawBoard(MicroDosAPI* api) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            int colorId = -1;
            colorId = ((r + c) % 2 == 0) ? 15 : 2;
            api->rect(c * 40, r * 40, 40, 40, colorId);
        }
    }
    api->flushGameMatrix();
}

static inline void initToledoBoard() {
    int initialLayout[] = {13, 9, 11, 15, 16, 12, 10, 14};
    int initialLayoutF[] = {13, 9, 11, 16, 15, 12, 10, 14};

    for (int i = 0; i <= 64; i++) b[i] = 0;

    for (int c = 0; c < 8; c++) {
        if (humanIsWhite) {
            b[1 + c]  = -initialLayout[c];  // Row 0: Black AI
            b[9 + c]  = -(c + 1);           // Row 1: Black AI Pawns
            b[49 + c] = (c + 1);            // Row 6: White Human Pawns
            b[57 + c] = initialLayout[c];   // Row 7: White Human
        } else {
            b[1 + c]  = initialLayoutF[c];  // Row 0: White AI
            b[9 + c]  = (c + 1);            // Row 1: White AI Pawns
            b[49 + c] = -(c + 1);           // Row 6: Black Human Pawns
            b[57 + c] = -initialLayoutF[c]; // Row 7: Black Human
        }
    }
}

static inline char getColChar(int colIdx) {
    switch (colIdx) {
        case 0: return 'A'; case 1: return 'B'; case 2: return 'C'; case 3: return 'D';
        case 4: return 'E'; case 5: return 'F'; case 6: return 'G'; case 7: return 'H';
        default: return '\0';
    }
}

static inline char getRowChar(int rowIdx) {
    switch (rowIdx) {
        case 0: return '8'; case 1: return '7'; case 2: return '6'; case 3: return '5';
        case 4: return '4'; case 5: return '3'; case 6: return '2'; case 7: return '1';
        default: return '\0';
    }
}

static inline void aiThinkAndRespond(MicroDosAPI* api);
static inline void executeMove(const char* moveStr, bool* playerTurn, MicroDosAPI* api);

int _start(int argc, char** argv, MicroDosAPI* api) {
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
	// QUIT
        if (key == '\x15' || key == 'Q' || key == 'q') {
            running = false;
            break;
        }
	// UNDO
        if (key == '\x12' || key == 'U' || key == 'u') {
            if (executeUndo()) {
                selectX = -1; selectY = -1;
                refreshBoard(api);
                api->delay(300);
            }
            continue;
        }
	// NEW
        if (key == '\x11') {
            humanIsWhite = true;
            playerTurn = true;
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
	// FLIP
        if (key == '\x13') {
            humanIsWhite = !humanIsWhite;
            selectX = -1; selectY = -1;
            for (int i = 1; i <= 32; i++) {
                if (pieceSprites[i] != 0) *(bool*)pieceSprites[i] = false;
            }

            initToledoBoard();
            clearUndoHistory();
            playerTurn = true;
            drawBoard(api);
            refreshBoard(api);

	    strcpy(textBuf, "      ");
	    bufLen = 6;
            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));

            if (!humanIsWhite) {
                saveUndoState();
                playerTurn = false;
                aiThinkAndRespond(api);
                refreshBoard(api);
                playerTurn = true;
            }

            api->delay(300);
            continue;
        }
        // Backspace
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
        // Enter
        if (key == '\n') {
            if (bufLen == 4) {
                moveStr[0] = textBuf[0];
                moveStr[1] = textBuf[1];
                moveStr[2] = textBuf[2];
                moveStr[3] = textBuf[3];
                moveStr[4] = '\0';

                saveUndoState();
		executeMove(moveStr, &playerTurn, api);
                strcpy(textBuf, "      ");
	       	bufLen = 6;
		memset(moveStr, 0, 5);
                selectX = -1;
                selectY = -1;
            }
	    api->delay(300);
	    continue;
        }
        // Alphanumeric
        else if (key >= 32 && key <= 126) {
            if (bufLen == 6 && strcmp(textBuf, "      ") == 0) {
                bufLen = 0;
                memset(textBuf, 0, sizeof(textBuf));
            }

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
                }
            } else {
                moveStr[0] = getColChar(selectX);
                moveStr[1] = getRowChar(selectY);
                moveStr[2] = getColChar(gridX);
                moveStr[3] = getRowChar(gridY);
                moveStr[4] = '\0';

                saveUndoState();
		executeMove(moveStr, &playerTurn, api);
                strcpy(textBuf, "      ");
	       	bufLen = 6;
		memset(moveStr, 0, 5);
                selectX = -1;
                selectY = -1;
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

static inline void aiThinkAndRespond(MicroDosAPI* api) {
    int bestMoveFrom = 0;
    int bestMoveTo   = 0;
    bool aiIsWhite   = !humanIsWhite;

    // Relative tracking: AI always wants the highest possible positive score
    int bestScore = -999999;

    for (int from = 1; from <= 64; from++) {
        bool isAiPiece = aiIsWhite ? (b[from] > 0) : (b[from] < 0);

        if (isAiPiece) {
            for (int to = 1; to <= 64; to++) {
                if (from == to) continue;

                // Prevent friendly fire
                if (b[to] != 0 && ((b[from] > 0 && b[to] > 0) || (b[from] < 0 && b[to] < 0))) continue;
                if (!isMoveValid(from, to)) continue;

                // Simulate Move
                int capturedPiece = b[to];
                b[to] = b[from];
                b[from] = 0;

                // Verify King safety
                if (isKingUnderAttack(aiIsWhite)) {
                    b[from] = b[to];
                    b[to] = capturedPiece;
                    continue;
                }

                // Calculate relative material layout
                int materialScore = 0;
                for (int i = 1; i <= 64; i++) {
                    int pieceVal = b[i];
                    if (pieceVal == 0) continue;

                    int pieceId = ABS(pieceVal);
                    if (pieceId > 16) pieceId = 16;

                    // PERSPECTIVE FIX: Positive points for AI pieces, negative points for opponent pieces
                    if (aiIsWhite) {
                        if (pieceVal > 0)  materialScore += pieceWeights[pieceId];
                        if (pieceVal < 0)  materialScore -= pieceWeights[pieceId];
                    } else {
                        if (pieceVal < 0)  materialScore += pieceWeights[pieceId];
                        if (pieceVal > 0)  materialScore -= pieceWeights[pieceId];
                    }
                }

                // Positional Control Bonus: Target center ring squares
                int toRow = (to - 1) / 8;
                int toCol = (to - 1) % 8;
                if (toRow >= 3 && toRow <= 4 && toCol >= 3 && toCol <= 4) {
                    materialScore += 15; // Always add a bonus for controlling the center
                }

                // AI always maximizes this relative score profile
                if (materialScore > bestScore) {
                    bestScore = materialScore;
                    bestMoveFrom = from;
                    bestMoveTo = to;
                }

                // Revert Simulation state
                b[from] = b[to];
                b[to] = capturedPiece;
            }
        }
    }

    // Execute the final chosen best move path
    if (bestMoveFrom != 0 && bestMoveTo != 0) {
        int fRow = (bestMoveFrom - 1) / 8;
        int fCol = (bestMoveFrom - 1) % 8;
        int tRow = (bestMoveTo - 1) / 8;
        int tCol = (bestMoveTo - 1) % 8;

        char cpuMove[5];
        cpuMove[0] = getColChar(fCol);
        cpuMove[1] = getRowChar(fRow);
        cpuMove[2] = getColChar(tCol);
        cpuMove[3] = getRowChar(tRow);
        cpuMove[4] = '\0';

        executeToledoMove(cpuMove);
    }
}

static inline void handlePawnPromotion() {
    // Check Row 0 (Indices 1 to 8) - White promoting on Black's back rank
    for (int col = 0; col < 8; col++) {
        int idx = col + 1;
        if (b[idx] >= 1 && b[idx] <= 8)   b[idx] = 15;  // White Pawn -> Standard White Queen
        if (b[idx] <= -1 && b[idx] >= -8) b[idx] = -15; // Black Pawn -> Standard Black Queen
    }

    // Check Row 7 (Indices 57 to 64) - Black promoting on White's back rank
    for (int col = 0; col < 8; col++) {
        int idx = 56 + col + 1;
        if (b[idx] >= 1 && b[idx] <= 8)   b[idx] = 15;  // White Pawn -> Standard White Queen
        if (b[idx] <= -1 && b[idx] >= -8) b[idx] = -15; // Black Pawn -> Standard Black Queen
    }
}

static inline void executeMove(const char* moveStr, bool* playerTurn, MicroDosAPI* api) {
    if (executeToledoMove(moveStr)) {
      handlePawnPromotion();
      refreshBoard(api);
      api->delay(200);

      if (countLegalMoves(!humanIsWhite) == 0) {
        if (isKingUnderAttack(!humanIsWhite)) {
          api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("YOUWIN"), STRING(" QUIT "));
        } else {
          api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("DRAW!!"), STRING(" QUIT "));
        }
        api->delay(2000);
        return;
      }

      playerTurn = (bool*)false;
      aiThinkAndRespond(api);
      handlePawnPromotion();
      refreshBoard(api);
      api->delay(200);
      playerTurn = (bool*)true;

      if (countLegalMoves(humanIsWhite) == 0) {
        if (isKingUnderAttack(humanIsWhite)) {
          api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("DEFEAT"), STRING(" QUIT "));
        } else {
          api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("DRAW!!"), STRING(" QUIT "));
        }
        api->delay(2000);
        return;
      }
    } else {
      undoHead = (undoHead - 1 + UNDO_DEPTH) % UNDO_DEPTH;
      if (undoCount > 0) undoCount--;
      refreshBoard(api);
    }

    api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
}

static inline bool isMoveValid(int fromIdx, int toIdx) {
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

    // PAWNS (IDs 1 to 8)
    if (pieceType >= 1 && pieceType <= 8) {
        bool movesUp = (p > 0) ? humanIsWhite : !humanIsWhite;

        if (movesUp) {
            if (dc == 0 && dr == -1 && t == 0) return true;
            if (dc == 0 && dr == -2 && fRow == 6 && b[fromIdx - 8] == 0 && t == 0) return true;
            if (absDc == 1 && dr == -1 && t != 0) return true;
        } else {
            if (dc == 0 && dr == 1 && t == 0) return true;
            if (dc == 0 && dr == 2 && fRow == 1 && b[fromIdx + 8] == 0 && t == 0) return true;
            if (absDc == 1 && dr == 1 && t != 0) return true;
        }
        return false;
    }

    //  KNIGHTS (IDs 9 and 10)
    if (pieceType == 9 || pieceType == 10) {
        return (absDc == 1 && absDr == 2) || (absDc == 2 && absDr == 1);
    }

    //  BISHOPS (IDs 11 and 12)
    if (pieceType == 11 || pieceType == 12) {
        if (absDc != absDr) return false;
        int c = fCol + sCol, r = fRow + sRow;
        while (c != tCol && r != tRow) {
            if (c < 0 || c > 7 || r < 0 || r > 7) return false;
            if (b[(r * 8) + c + 1] != 0) return false;
            c += sCol; r += sRow;
        }
        return true;
    }

    //  ROOKS (IDs 13 and 14)
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

    //  QUEENS (ID 15)
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

    //  KINGS (ID 16)
    if (pieceType == 16) {
        return (absDc <= 1 && absDr <= 1);
    }

    return false;
}
