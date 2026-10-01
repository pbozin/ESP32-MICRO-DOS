#include "microdos_api.h"

#define KEY_LEFT  '\x11'
#define KEY_RIGHT '\x15'
#define KEY_ESC   '\x13'

extern "C" int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;

    int pSize = api->termWidth / 6;
    int bWidth = api->termWidth / 10;
    int bHeight = api->termHeight / 30;

    int p = (api->termWidth / 2) - (pSize / 2);
    int q = api->termHeight - 20;

    int x = api->termWidth / 2;
    int y = api->termHeight / 2;
    int u = 4;
    int v = -4;

    api->setFKeys(STRING("  <-  "), STRING("      "), STRING("  ESC "), STRING("      "), STRING("  ->  "));
    api->clear();
    api->color(GREEN);
    api->println(STRING("BRICKS STARTING"));
    api->delay(1000);
    api->clear();

    for (int i = 0; i < 10 * 5; i++) {
        api->poke(i, 1);
        int r = i / 10;
        int c = i % 10;
        int a = c * bWidth;
        int b = r * bHeight;
        api->rect(a, b, bWidth - 1, bHeight - 1, r + 3);
    }

    api->delay(500);
    int running = 1;

    while (running) {
        int k = api->inkey();

        if (k == KEY_LEFT) {
            api->rect(p, q, pSize, 6, BLACK);
            p -= 12;
            if (p < 0) p = 0;
        }
        else if (k == KEY_RIGHT) {
            api->rect(p, q, pSize, 6, BLACK);
            p += 12;
            if (p > api->termWidth - pSize) p = api->termWidth - pSize;
        }
        else if (k == KEY_ESC) {
            running = 0;
            break;
        }

        api->rect(x - 3, y - 3, 6, 6, BLACK);

        x += u;
        y += v;

        if (x < 4) {
            x = 4;
            u = 0 - u;
            api->beep(800, 10);
        } else if (x > api->termWidth - 4) {
            x = api->termWidth - 4;
            u = 0 - u;
            api->beep(800, 10);
        }

        if (y < 4) {
            y = 4;
            v = 0 - v;
            api->beep(800, 10);
        }

        if (y < bHeight * 5) {
            int r = y / bHeight;
            int c = x / bWidth;
            int n = (r * 10) + c;

            if (n >= 0 && n <= 49) {
                if (api->peek(n) == 1) {
                    api->poke(n, 0);
                    v = 0 - v;
                    api->beep(1200, 15);

                    int a = c * bWidth;
                    int b = r * bHeight;
                    api->rect(a, b, bWidth, bHeight, BLACK);
                }
            }
        }

        if (y >= q && y <= q + 6) {
            if (x >= p && x <= (p + pSize)) {
                y = q - 1;
                v = 0 - v;
                api->beep(1000, 20);
            }
        }

        api->rect(p, q, pSize, 6, BLUE);
        api->rect(x - 3, y - 3, 6, 6, YELLOW);

        if (y > api->termHeight - 10) {
            api->beep(150, 600);
            api->clear();
            api->color(RED);
            api->println("\nGAME OVER!");
            api->delay(2000);
            running = 0;
        }
        api->delay(8);
    }

    api->clearFKeys();
    api->color(GREEN);
    api->clear();
    return 0;
}
