#include "microdos_api.h"

#define KEY_LEFT  '\x11'
#define KEY_RIGHT '\x15'
#define KEY_ESC   '\x13'

extern "C" int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;

    int pSize = getTermWidth() / 6;
    int bWidth = getTermWidth() / 10;
    int bHeight = getTermHeight() / 30;

    int p = (getTermWidth() / 2) - (pSize / 2);
    int q = getTermHeight() - 20;

    int x = getTermWidth() / 2;
    int y = getTermHeight() / 2;
    int u = 4;
    int v = -4;

    setFKeys(STRING("  <-  "), STRING("      "), STRING("  ESC "), STRING("      "), STRING("  ->  "));
    termClear();
    setColor(GREEN);
    termPrintln(STRING("BRICKS STARTING"));
    delayMs(1000);
    termClear();

    for (int i = 0; i < 10 * 5; i++) {
        poke(i, 1);
        int r = i / 10;
        int c = i % 10;
        int a = c * bWidth;
        int b = r * bHeight;
        fillRect(a, b, bWidth - 1, bHeight - 1, r + 3);
    }

    delayMs(500);
    int running = 1;

    while (running) {
        int k = getKey();

        if (k == KEY_LEFT) {
            fillRect(p, q, pSize, 6, BLACK);
            p -= 12;
            if (p < 0) p = 0;
        }
        else if (k == KEY_RIGHT) {
            fillRect(p, q, pSize, 6, BLACK);
            p += 12;
            if (p > getTermWidth() - pSize) p = getTermWidth() - pSize;
        }
        else if (k == KEY_ESC) {
            running = 0;
            break;
        }

        fillRect(x - 3, y - 3, 6, 6, BLACK);

        x += u;
        y += v;

        if (x < 4) {
            x = 4;
            u = 0 - u;
            playSound(800, 10);
        } else if (x > getTermWidth() - 4) {
            x = getTermWidth() - 4;
            u = 0 - u;
            playSound(800, 10);
        }

        if (y < 4) {
            y = 4;
            v = 0 - v;
            playSound(800, 10);
        }

        if (y < bHeight * 5) {
            int r = y / bHeight;
            int c = x / bWidth;
            int n = (r * 10) + c;

            if (n >= 0 && n <= 49) {
                if (peek(n) == 1) {
                    poke(n, 0);
                    v = 0 - v;
                    playSound(1200, 15);

                    int a = c * bWidth;
                    int b = r * bHeight;
                    fillRect(a, b, bWidth, bHeight, BLACK);
                }
            }
        }

        if (y >= q && y <= q + 6) {
            if (x >= p && x <= (p + pSize)) {
                y = q - 1;
                v = 0 - v;
                playSound(1000, 20);
            }
        }

        fillRect(p, q, pSize, 6, BLUE);
        fillRect(x - 3, y - 3, 6, 6, YELLOW);

        if (y > getTermHeight() - 10) {
            playSound(150, 600);
            termClear();
            setColor(RED);
            termPrintln("\nGAME OVER!");
            delayMs(2000);
            running = 0;
        }
        delayMs(8);
    }

    clearFKeys();
    setColor(GREEN);
    termClear();
    return 0;
}
