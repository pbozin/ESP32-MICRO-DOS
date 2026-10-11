#include <stdio.h>
#include <picoc_esp32.h>
#include <graphics.h>

#define KEY_LEFT  0x11
#define KEY_RIGHT 0x15
#define KEY_ESC   0x13

void main() {
    int termWidth = getTermWidth();
    int termHeight = getTermHeight();

    int pSize = termWidth / 6;
    int bWidth = termWidth / 10;
    int bHeight = termHeight / 30;

    int p = (termWidth / 2) - (pSize / 2);
    int q = termHeight - 20;

    int x = termWidth / 2;
    int y = termHeight / 2;
    int u = 4;
    int v = -4;
    
    int i;
    int running;

    clearScreen(); 
    setFKeys("  <-  ", "      ", "  ESC ", "      ", "  ->  ");
    setColor(GREEN);
    printf("BRICKS STARTING\n");
    delay(1000); 
    clearScreen();

    for (i = 0; i < 10 * 5; i++) {
        poke(i, 1);
        int r = i / 10;
        int c = i % 10;
        int a = c * bWidth;
        int b = r * bHeight;
        fillRect(a, b, bWidth - 1, bHeight - 1, r + 3);
    }

    delay(500);
    running = 1;

    fcntl(0, F_SETFL, O_NONBLOCK); 

    while (running) {
        int k = getchar();

        if (k == KEY_LEFT) {
            fillRect(p, q, pSize, 6, BLACK);
            p -= 12;
            if (p < 0) p = 0;
        }
        else if (k == KEY_RIGHT) {
            fillRect(p, q, pSize, 6, BLACK);
            p += 12;
            if (p > termWidth - pSize) p = termWidth - pSize;
        }
        else if (k == KEY_ESC) {
            running = 0;
            break;
        }

        fillCircle(x - 3, y - 3, 3, BLACK);

        x += u;
        y += v;

        if (x < 4) {
            x = 4;
            u = 0 - u;
            beep(800, 10); 
        } else if (x > termWidth - 4) {
            x = termWidth - 4;
            u = 0 - u;
            beep(800, 10);
        }

        if (y < 4) {
            y = 4;
            v = 0 - v;
            beep(800, 10);
        }

        if (y < bHeight * 5) {
            int r = y / bHeight;
            int c = x / bWidth;
            int n = (r * 10) + c;

            if (n >= 0 && n <= 49) {
                if (peek(n) == 1) {
                    poke(n, 0);
                    v = 0 - v;
                    beep(1200, 15);

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
                beep(1000, 20);
            }
        }

        fillRect(p, q, pSize, 6, BLUE);
        fillCircle(x - 3, y - 3, 3, YELLOW);

        if (y > termHeight - 10) {
            beep(150, 600);
            clearScreen();
            setColor(RED);
            printf("\nGAME OVER!\n");
            delay(2000);
            running = 0;
        }
        
        delay(8); 
    }

    fcntl(0, F_SETFL, O_RDONLY); 
    clearFKeys();
    clearScreen();
    setColor(GREEN);
}

