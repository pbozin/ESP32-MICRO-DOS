#include "microdos_api.h"
#include <cstring>
#include <stdbool.h>

#define UNDO_DEPTH 6
#define ABS(x) ((x) < 0 ? -(x) : (x))

static uint32_t whiteCastleKingsSide  __attribute__((aligned(4))) = 1;
static uint32_t whiteCastleQueensSide __attribute__((aligned(4))) = 1;
static uint32_t blackCastleKingsSide  __attribute__((aligned(4))) = 1;
static uint32_t blackCastleQueensSide __attribute__((aligned(4))) = 1;
static int32_t bufLen __attribute__((aligned(4))) = 6;
static int32_t undoHead __attribute__((aligned(4))) = 0;
static int32_t undoCount __attribute__((aligned(4))) = 0;
static uint32_t humanIsWhite __attribute__((aligned(4))) = 1;

static int32_t b[68] __attribute__((aligned(4)));
static int32_t undoHistory[UNDO_DEPTH][68] __attribute__((aligned(4)));

static bool isKingUnderAttack(bool isWhiteKing);

INLINE ALWAYS bool checkPawnMove(int32_t fromIdx, int32_t toIdx, int32_t p, int32_t t, int32_t fRow, int32_t dc, int32_t dr, int32_t absDc) {
    bool movesUp = (p > 0) ? (humanIsWhite != 0) : (humanIsWhite == 0);

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

INLINE ALWAYS bool checkSlidingMove(int32_t fCol, int32_t fRow, int32_t tCol, int32_t tRow, int32_t sCol, int32_t sRow) {
    int32_t c = fCol + sCol;
    int32_t r = fRow + sRow;
    while (c != tCol || r != tRow) {
        if (c < 0 || c > 7 || r < 0 || r > 7) return false;

        int32_t boardIdx = (r * 8) + c + 1;
        if (boardIdx < 1 || boardIdx > 64) return false;

        if (b[boardIdx] != 0) return false;
        c += sCol;
        r += sRow;
    }
    return true;
}

INLINE ALWAYS int32_t findKing(bool whiteKing) {
    int32_t targetId = whiteKing ? 16 : -16;
    for (int32_t i = 1; i <= 64; i++) {
        if (b[i] == targetId) return i;
    }
    return 0;
}

INLINE ALWAYS bool checkKingMove(int32_t fromIdx, int32_t p, int32_t dc, int32_t dr, int32_t absDc, int32_t absDr) {
    if (absDc <= 1 && absDr <= 1) return true;

    if (dr == 0 && absDc == 2) {
        bool isWhiteKing = (p > 0);

        if (isKingUnderAttack(isWhiteKing)) return false;

        if (dc == 2) {
            uint32_t rights = isWhiteKing ? whiteCastleKingsSide : blackCastleKingsSide;
            if (rights == 0) return false;
            if (b[fromIdx + 1] != 0 || b[fromIdx + 2] != 0) return false;

            int32_t backup = b[fromIdx + 1];
            b[fromIdx + 1] = p; b[fromIdx] = 0;
            bool passThroughCheck = isKingUnderAttack(isWhiteKing);
            b[fromIdx] = p; b[fromIdx + 1] = backup;

            return !passThroughCheck;
        }

        if (dc == -2) {
            uint32_t rights = isWhiteKing ? whiteCastleQueensSide : blackCastleQueensSide;
            if (rights == 0) return false;
            if (b[fromIdx - 1] != 0 || b[fromIdx - 2] != 0 || b[fromIdx - 3] != 0) return false;

            int32_t backup = b[fromIdx - 1];
            b[fromIdx - 1] = p; b[fromIdx] = 0;
            bool passThroughCheck = isKingUnderAttack(isWhiteKing);
            b[fromIdx] = p; b[fromIdx - 1] = backup;

            return !passThroughCheck;
        }
    }
    return false;
}

static bool isMoveValid(int32_t fromIdx, int32_t toIdx) {
    if (fromIdx < 1 || fromIdx > 64 || toIdx < 1 || toIdx > 64) return false;
    if (fromIdx == toIdx) return false;

    int32_t p = b[fromIdx];
    int32_t t = b[toIdx];

    if (p == 0) return false;
    if (t != 0 && ((p > 0 && t > 0) || (p < 0 && t < 0))) return false;

    int32_t fCol = (fromIdx - 1) % 8;
    int32_t fRow = (fromIdx - 1) / 8;
    int32_t tCol = (toIdx - 1) % 8;
    int32_t tRow = (toIdx - 1) / 8;

    int32_t dc = tCol - fCol;
    int32_t dr = tRow - fRow;
    int32_t absDc = ABS(dc);
    int32_t absDr = ABS(dr);

    int32_t sCol = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
    int32_t sRow = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);

    int32_t pieceType = ABS(p);

    if (pieceType >= 1 && pieceType <= 8) {
        return checkPawnMove(fromIdx, toIdx, p, t, fRow, dc, dr, absDc);
    }
    if (pieceType == 9 || pieceType == 10) {
        return (absDc == 1 && absDr == 2) || (absDc == 2 && absDr == 1);
    }
    if (pieceType == 11 || pieceType == 12) { // Bishop
        if (absDc != absDr) return false;
        return checkSlidingMove(fCol, fRow, tCol, tRow, sCol, sRow);
    }
    if (pieceType == 13 || pieceType == 14) { // Rook
        if (dc != 0 && dr != 0) return false;
        return checkSlidingMove(fCol, fRow, tCol, tRow, sCol, sRow);
    }
    if (pieceType == 15) { // Queen
        if (absDc != absDr && dc != 0 && dr != 0) return false;
        return checkSlidingMove(fCol, fRow, tCol, tRow, sCol, sRow);
    }
    if (pieceType == 16) { // King
        return checkKingMove(fromIdx, p, dc, dr, absDc, absDr);
    }

    return false;
}

static bool isKingUnderAttack(bool isWhiteKing) {
    int32_t kingIdx = findKing(isWhiteKing);
    if (kingIdx < 1 || kingIdx > 64) return false;

    for (int32_t attackerIdx = 1; attackerIdx <= 64; attackerIdx++) {
        int32_t p = b[attackerIdx];
        if (p == 0) continue;

        if ((isWhiteKing && p < 0) || (!isWhiteKing && p > 0)) {
            if (isMoveValid(attackerIdx, kingIdx)) {
                return true;
            }
        }
    }
    return false;
}

INLINE int32_t countLegalMoves(bool isWhiteTurn) {
    int32_t legalCount = 0;

    for (int32_t from = 1; from <= 64; from++) {
        int32_t p = b[from];
        if (p == 0) continue;

        if (isWhiteTurn && p < 0) continue;
        if (!isWhiteTurn && p > 0) continue;

        for (int32_t to = 1; to <= 64; to++) {
            if (from == to) continue;

            if (b[to] != 0) {
                if (isWhiteTurn && b[to] > 0) continue;
                if (!isWhiteTurn && b[to] < 0) continue;
            }

            if (!isMoveValid(from, to)) continue;

            int32_t backupTo = b[to];
            b[to] = b[from];
            b[from] = 0;

            bool selfCheck = isKingUnderAttack(isWhiteTurn);

            b[from] = b[to];
            b[to] = backupTo;

            if (!selfCheck) {
                legalCount++;
            }
        }
    }
    return legalCount;
}

INLINE ALWAYS bool executeToledoMove(const char* moveStr) {
    if (!moveStr || moveStr[0] == '\0') return false;
    int32_t fromCol = moveStr[0] - 'A';
    int32_t fromRow = '8' - moveStr[1];
    int32_t toCol   = moveStr[2] - 'A';
    int32_t toRow   = '8' - moveStr[3];

    if (fromCol < 0 || fromCol > 7 || fromRow < 0 || fromRow > 7) return false;
    if (toCol < 0 || toCol > 7 || toRow < 0 || toRow > 7) return false;

    int32_t fromIdx = (fromRow * 8) + fromCol + 1;
    int32_t toIdx   = (toRow * 8) + toCol + 1;

    if (!isMoveValid(fromIdx, toIdx)) return false;

    bool isWhiteMove = (b[fromIdx] > 0);
    int32_t pieceType = ABS(b[fromIdx]);

    int32_t capturedPiece = b[toIdx];
    b[toIdx] = b[fromIdx];
    b[fromIdx] = 0;

    bool leavesKingInCheck = isKingUnderAttack(isWhiteMove);

    b[fromIdx] = b[toIdx];
    b[toIdx] = capturedPiece;

    if (leavesKingInCheck) return false;

    if (pieceType == 16 && ABS(toCol - fromCol) == 2) {
        if (toCol - fromCol == 2) {
            int32_t rookFrom = toIdx + 1;
            int32_t rookTo = toIdx - 1;
            b[rookTo] = b[rookFrom];
            b[rookFrom] = 0;
        }
        else if (toCol - fromCol == -2) {
            int32_t rookFrom = toIdx - 2;
            int32_t rookTo = toIdx + 1;
            b[rookTo] = b[rookFrom];
            b[rookFrom] = 0;
        }
    }

    b[toIdx] = b[fromIdx];
    b[fromIdx] = 0;

    if (pieceType == 16) {
        if (isWhiteMove) { whiteCastleKingsSide = 0; whiteCastleQueensSide = 0; }
        else             { blackCastleKingsSide = 0; blackCastleQueensSide = 0; }
    }
    else if (pieceType == 13 || pieceType == 14) {
        int32_t whiteKingsideRook  = (humanIsWhite != 0) ? 64 : 8;
        int32_t whiteQueensideRook = (humanIsWhite != 0) ? 57 : 1;
        int32_t blackKingsideRook  = (humanIsWhite != 0) ? 8  : 64;
        int32_t blackQueensideRook = (humanIsWhite != 0) ? 1  : 57;

        if (isWhiteMove) {
            if (fromIdx == whiteKingsideRook)  whiteCastleKingsSide = 0;
            if (fromIdx == whiteQueensideRook) whiteCastleQueensSide = 0;
        } else {
            if (fromIdx == blackKingsideRook)  blackCastleKingsSide = 0;
            if (fromIdx == blackQueensideRook) blackCastleQueensSide = 0;
        }
    }

    return true;
}

static const int32_t pieceWeights[] __attribute__((aligned(4))) = {
    0, 100, 100, 100, 100, 100, 100, 100, 100,
    300, 300, 300, 300, 500, 500, 900, 9999
};

INLINE ALWAYS int32_t calculateTotalHangingPenalty(bool aiIsWhite) {
    int32_t totalPenalty = 0;

    for (int32_t targetIdx = 1; targetIdx <= 64; targetIdx++) {
        int32_t pieceVal = b[targetIdx];
        if (pieceVal == 0) continue;

        bool isAiPiece = aiIsWhite ? (pieceVal > 0) : (pieceVal < 0);
        if (!isAiPiece) continue;

        for (int32_t attackerIdx = 1; attackerIdx <= 64; attackerIdx++) {
            int32_t enemyPiece = b[attackerIdx];
            if (enemyPiece == 0) continue;

            bool isEnemy = aiIsWhite ? (enemyPiece < 0) : (enemyPiece > 0);
            if (isEnemy && isMoveValid(attackerIdx, targetIdx)) {
                int32_t pId = ABS(pieceVal);
                if (pId > 16) pId = 16;
                totalPenalty += pieceWeights[pId];
                break;
            }
        }
    }
    return totalPenalty;
}

INLINE ALWAYS int32_t evaluateMaterialLayout(bool aiIsWhite) {
    int32_t materialScore = 0;
    for (int32_t i = 1; i <= 64; i++) {
        int32_t pieceVal = b[i];
        if (pieceVal == 0) continue;

        int32_t pieceId = ABS(pieceVal);
        if (pieceId > 16) pieceId = 16;

        if (aiIsWhite) {
            materialScore += (pieceVal > 0) ? pieceWeights[pieceId] : -pieceWeights[pieceId];
        } else {
            materialScore += (pieceVal < 0) ? pieceWeights[pieceId] : -pieceWeights[pieceId];
        }
    }
    return materialScore;
}

INLINE ALWAYS bool isSquareDefendedByEnemy(int32_t targetIdx, bool aiIsWhite) {
    if (targetIdx < 1 || targetIdx > 64) return false;

    for (int32_t attackerIdx = 1; attackerIdx <= 64; attackerIdx++) {
        int32_t enemyPiece = b[attackerIdx];
        if (enemyPiece == 0) continue;

        bool isEnemy = aiIsWhite ? (enemyPiece < 0) : (enemyPiece > 0);
        if (isEnemy) {
            if (isMoveValid(attackerIdx, targetIdx)) {
                return true;
            }
        }
    }
    return false;
}

INLINE ALWAYS void aiThinkAndRespond(MicroDosAPI* api) {
    int32_t bestMoveFrom = 0;
    int32_t bestMoveTo   = 0;
    bool aiIsWhite   = (humanIsWhite == 0);
    int32_t bestScore    = -999999;

    int32_t initialHangingPenalty = calculateTotalHangingPenalty(aiIsWhite);

    for (int32_t from = 1; from <= 64; from++) {
        bool isAiPiece = aiIsWhite ? (b[from] > 0) : (b[from] < 0);
        if (!isAiPiece) continue;

        for (int32_t to = 1; to <= 64; to++) {
            if (from == to) continue;

            if (b[to] != 0 && ((b[from] > 0 && b[to] > 0) || (b[from] < 0 && b[to] < 0))) continue;
            if (!isMoveValid(from, to)) continue;

            int32_t capturedPiece = b[to];
            b[to] = b[from];
            b[from] = 0;

            if (isKingUnderAttack(aiIsWhite)) {
                b[from] = b[to];
                b[to] = capturedPiece;
                continue;
            }

            int32_t currentMoveScore = evaluateMaterialLayout(aiIsWhite);

            if (isSquareDefendedByEnemy(to, aiIsWhite)) {
                int32_t movingPieceId = ABS(b[to]);
                if (movingPieceId > 16) movingPieceId = 16;
                currentMoveScore -= pieceWeights[movingPieceId];
            }

            int32_t postHangingPenalty = calculateTotalHangingPenalty(aiIsWhite);
            int32_t dangerResolvedBonus = initialHangingPenalty - postHangingPenalty;
            currentMoveScore += dangerResolvedBonus;

            int32_t toRow = (to - 1) / 8;
            int32_t toCol = (to - 1) % 8;
            if (toRow >= 3 && toRow <= 4 && toCol >= 3 && toCol <= 4) {
                currentMoveScore += 15;
            }

            if (currentMoveScore > bestScore) {
                bestScore = currentMoveScore;
                bestMoveFrom = from;
                bestMoveTo = to;
            }

            b[from] = b[to];
            b[to] = capturedPiece;
        }
    }
    if (bestMoveFrom != 0 && bestMoveTo != 0) {
        int32_t fRow = (bestMoveFrom - 1) / 8;
        int32_t fCol = (bestMoveFrom - 1) % 8;
        int32_t tRow = (bestMoveTo - 1) / 8;
        int32_t tCol = (bestMoveTo - 1) % 8;

	char cpuMove[8] __attribute__((aligned(4)));
        cpuMove[0] = (char)('A' + fCol);
        cpuMove[1] = (char)('8' - fRow);
        cpuMove[2] = (char)('A' + tCol);
        cpuMove[3] = (char)('8' - tRow);
        cpuMove[4] = '\0';

        executeToledoMove(cpuMove);
    }
}

INLINE void handlePawnPromotion() {
    for (int32_t col = 0; col < 8; col++) {
        int32_t idx = col + 1;
        if (b[idx] >= 1 && b[idx] <= 8)   b[idx] = 15;
        if (b[idx] <= -1 && b[idx] >= -8) b[idx] = -15;
    }

    for (int32_t col = 0; col < 8; col++) {
        int32_t idx = 56 + col + 1;
        if (b[idx] >= 1 && b[idx] <= 8)   b[idx] = 15;
        if (b[idx] <= -1 && b[idx] >= -8) b[idx] = -15;
    }
}

static uint32_t pieceSprites[36] __attribute__((aligned(4)));

INLINE ALWAYS void refreshBoard(MicroDosAPI* api) {
    for (int32_t r = 0; r < 8; r++) {
        for (int32_t c = 0; c < 8; c++) {
            int32_t boardIdx = (r * 8) + c + 1;
            int32_t pieceVal = b[boardIdx];

            if (pieceVal != 0) {
                  int32_t spriteIdx = (pieceVal > 0) ? pieceVal : (-pieceVal + 16);
                  uint32_t handle = pieceSprites[spriteIdx];
                  if (handle != 0) {
                    api->drawSprite(handle, c * 40, r * 40);
                  }
            }
        }
    }
}

INLINE ALWAYS void executeMove(const char* moveStr, MicroDosAPI* api) {
    if (executeToledoMove(moveStr)) {
      handlePawnPromotion();
      refreshBoard(api);
      api->delay(200);

      if (countLegalMoves(humanIsWhite == 0) == 0) {
        if (isKingUnderAttack(humanIsWhite == 0)) {
          api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("YOUWIN"), STRING(" QUIT "));
        } else {
          api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("DRAW!!"), STRING(" QUIT "));
        }
        api->delay(2000);
        return;
      }

      aiThinkAndRespond(api);
      handlePawnPromotion();
      refreshBoard(api);
      api->delay(200);

      if (countLegalMoves(humanIsWhite != 0) == 0) {
        if (isKingUnderAttack(humanIsWhite != 0)) {
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

INLINE ALWAYS void drawBoard(MicroDosAPI* api) {
    for (int32_t r = 0; r < 8; r++) {
        for (int32_t c = 0; c < 8; c++) {
            int32_t colorId = -1;
            colorId = ((r + c) % 2 == 0) ? 15 : 2;
            api->rect(c * 40, r * 40, 40, 40, colorId);
        }
    }
    api->flushGameMatrix();
}

INLINE ALWAYS void initToledoBoard() {
    static const int32_t initialLayout[] __attribute__((aligned(4))) = {13, 9, 11, 15, 16, 12, 10, 14};
    static const int32_t initialLayoutF[] __attribute__((aligned(4))) = {13, 9, 11, 16, 15, 12, 10, 14};

    for (int32_t i = 0; i < 68; i++) b[i] = 0;

    whiteCastleQueensSide = 1;
    whiteCastleKingsSide = 1;
    blackCastleQueensSide = 1;
    blackCastleKingsSide = 1;

    for (int32_t c = 0; c < 8; c++) {
        if (humanIsWhite != 0) {
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

INLINE ALWAYS void saveUndoState() {
    memcpy(undoHistory[undoHead], b, sizeof(b));
    undoHead = (undoHead + 1) % UNDO_DEPTH;
    if (undoCount < UNDO_DEPTH) undoCount++;
}

INLINE ALWAYS bool executeUndo() {
    if (undoCount <= 0) return false;
    undoHead = (undoHead - 1 + UNDO_DEPTH) % UNDO_DEPTH;
    memcpy(b, undoHistory[undoHead], sizeof(b));
    undoCount--;
    return true;
}

INLINE ALWAYS void clearUndoHistory() {
    undoHead = 0;
    undoCount = 0;
}

INLINE ALWAYS void cacheAllChessSprites(MicroDosAPI* api) {
    const char* chessSpriteFiles[12] = {
        STRING("Chess_plt60.spr"), // 0: Pawn
        STRING("Chess_nlt60.spr"), // 1: Knight
        STRING("Chess_blt60.spr"), // 2: Bishop
        STRING("Chess_rlt60.spr"), // 3: Rook
        STRING("Chess_qlt60.spr"), // 4: Queen
        STRING("Chess_klt60.spr"), // 5: King
        STRING("Chess_pdt60.spr"), // 6: Dark Pawn
        STRING("Chess_ndt60.spr"), // 7: Dark Knight
        STRING("Chess_bdt60.spr"), // 8: Dark Bishop
        STRING("Chess_rdt60.spr"), // 9: Dark Rook
        STRING("Chess_qdt60.spr"), // 10: Dark Queen
        STRING("Chess_kdt60.spr")  // 11: Dark King
    };

    for (int32_t i = 1; i <= 32; i++) {
        int32_t fileIdx = -1;

        if (i >= 1 && i <= 8)        fileIdx = 0;  // White Pawns
        else if (i == 9 || i == 10)  fileIdx = 1;  // White Knights
        else if (i == 11 || i == 12) fileIdx = 2;  // White Bishops
        else if (i == 13 || i == 14) fileIdx = 3;  // White Rooks
        else if (i == 15)            fileIdx = 4;  // White Queen
        else if (i == 16)            fileIdx = 5;  // White King
        else if (i >= 17 && i <= 24) fileIdx = 6;  // Black Pawns
        else if (i == 25 || i == 26) fileIdx = 7;  // Black Knights
        else if (i == 27 || i == 28) fileIdx = 8;  // Black Bishops
        else if (i == 29 || i == 30) fileIdx = 9;  // Black Rooks
        else if (i == 31)            fileIdx = 10; // Black Queen
        else if (i == 32)            fileIdx = 11; // Black King

        if (fileIdx != -1) {
            pieceSprites[i] = api->createSprite(chessSpriteFiles[fileIdx]);
        }
    }
}

extern "C" int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;
    if (!api->initGameMatrix()) return -1;

    api->clear();
    api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
    cacheAllChessSprites(api);
    initToledoBoard();
    drawBoard(api);
    refreshBoard(api);

    int32_t selectX = -1, selectY = -1;
    bool running = true;
    bool playerTurn = true;

    static char moveStr[8] __attribute__((aligned(4))) = "";
    static char textBuf[8] __attribute__((aligned(4))) = "      ";
    static char blankBuf[8] __attribute__((aligned(4))) = "      ";

    while (running) {
        int32_t key = api->inkey();
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
            humanIsWhite = 1;
            playerTurn = true;
            selectX = -1; selectY = -1;
            initToledoBoard();
            clearUndoHistory();
            memcpy(textBuf, blankBuf, sizeof(blankBuf));
            bufLen = 6;
            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), STRING("      "), STRING(" QUIT "));
            drawBoard(api);
            for (int32_t i = 1; i <= 32; i++) {
              if (pieceSprites[i] != 0) { *(bool*)pieceSprites[i] = false; }
            }
            refreshBoard(api);
            api->delay(300);
            continue;
        }
        // FLIP
        if (key == '\x13') {
            humanIsWhite = (humanIsWhite == 0) ? 1 : 0;
            selectX = -1; selectY = -1;
            for (int32_t i = 1; i <= 32; i++) {
                if (pieceSprites[i] != 0) *(bool*)pieceSprites[i] = false;
            }

            initToledoBoard();
            clearUndoHistory();
            playerTurn = true;
            drawBoard(api);
            refreshBoard(api);

            memcpy(textBuf, blankBuf, sizeof(blankBuf));
            bufLen = 6;
            api->setFKeys(STRING(" NEW  "), STRING(" UNDO "), STRING(" FLIP "), textBuf, STRING(" QUIT "));

            if (humanIsWhite == 0) {
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
                    memcpy(textBuf, blankBuf, sizeof(blankBuf));
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
                memcpy(textBuf, blankBuf, sizeof(blankBuf));
                bufLen = 6;
                executeMove(moveStr, api);
                memset(moveStr, 0, sizeof(moveStr));
                selectX = -1;
                selectY = -1;
            }
            api->delay(300);
            continue;
        }
        // Alphanumeric
        else if (key >= 32 && key <= 126) {
            if (bufLen == 6 && strcmp(textBuf, blankBuf) == 0) {
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
            int32_t gridX = touch.x / 40;
            int32_t gridY = touch.y / 40;

            if (selectX == -1) {
                int32_t selectIdx = (gridY * 8) + gridX + 1;
                int32_t piece = b[selectIdx];

                bool isHumanPiece = (humanIsWhite != 0) ? (piece > 0) : (piece < 0);

                if (piece != 0 && playerTurn && isHumanPiece) {
                    selectX = gridX;
                    selectY = gridY;
                }
            } else {
                moveStr[0] = (char)('A' + selectX);
                moveStr[1] = (char)('8' - selectY);
                moveStr[2] = (char)('A' + gridX);
                moveStr[3] = (char)('8' - gridY);
                moveStr[4] = '\0';

                saveUndoState();
                memcpy(textBuf, blankBuf, sizeof(blankBuf));
                bufLen = 6;
                executeMove(moveStr, api);
                memset(moveStr, 0, sizeof(moveStr));
                selectX = -1;
                selectY = -1;
            }
            api->delay(300);
        }
        api->delay(30);
    }

    api->clearFKeys();
    for (int32_t i = 1; i <= 32; i++) {
        if (pieceSprites[i] != 0) api->freeSprite(pieceSprites[i]);
    }
    api->closeGameMatrix();
    api->delay(100);
    return 0;
}
