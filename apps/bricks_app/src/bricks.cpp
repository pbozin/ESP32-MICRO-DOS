// main.c
#include "microdos_api.h"

// Explicit symbol mapping for keyboard array grid
#define KEY_O 79   // Left movement toggle key
#define KEY_P 80   // Right movement toggle key
#define KEY_Q 113  // "q" key to quit back to DOS prompt

extern "C" int _start(int argc, char** argv, MicroDosAPI* api) {
    // Initialize Variables & Screen
    int p = 140;       // Paddle X position
    int q = 300;       // Paddle Y position
    int x = 160;       // Ball X position
    int y = 150;       // Ball Y position
    int u = 4;         // Ball horizontal speed vector
    int v = -4;        // Ball vertical speed vector
    
    api->clear();
    api->color(GREEN); // Set text to default system
    api->println(STRING("BRICKS STARTING"));
    api->delay(1000);
    api->clear();
    
    // Initialize and draw the brick grid once (Allocated to address 0-19)
    for (int i = 0; i < 10 * 5; i++) {
        api->poke(i, 1);
        int r = i / 10;
        int c = i % 10;
        int a = c * 32;
        int b = r * 15;
        api->rect(a, b, 30, 12, r + 3); // Draw Magenta Bricks 
    }
    
    // Wipe baseline boot banner to clear the field context
    api->delay(500);
    api->rect(0, 240, 320, 40, BLACK); 
    
    int running = 1;
    
    // Main Game Loop Context
    while (running) {
        api->flushGameMatrix();        
        int k = api->inkey();
        
        if (k == KEY_O) { // Move Paddle Left
            api->rect(p, q, 60, 6, BLACK); // Erase old full paddle bounds cleanly
            p -= 12;
            if (p < 0) p = 0;
        } 
        else if (k == KEY_P) { // Move Paddle Right
            api->rect(p, q, 60, 6, BLACK); // Erase old full paddle bounds cleanly
            p += 12;
            if (p > 260) p = 260;
        } 
        else if (k == KEY_Q) { // Exit back to microDOS shell
            running = 0;
            break;
        }
        
        // Erase old ball vector trail trace
        api->rect(x - 3, y - 3, 6, 6, BLACK);
        
        // Advance ball metrics positioning physics
        x += u;
        y += v;
        
        // Left & Right screen boundary wall reflections
        if (x < 4) {
            x = 4;
            u = 0 - u;
            api->beep(800, 10);
        } else if (x > 312) {
            x = 312;
            u = 0 - u;
            api->beep(800, 10);
        }
        
        // Ceiling screen boundary wall reflections
        if (y < 4) {
            y = 4;
            v = 0 - v;
            api->beep(800, 10);
        }
        
        // Brick Matrix Collision Layer Tracking
        if (y < 40 * 5) {
            int r = y / 15;
            int c = x / 32;
            int n = (r * 10) + c;
            
            if (n >= 0 && n <= 49) {
                if (api->peek(n) == 1) { // Hit confirmed
                    api->poke(n, 0);     // Mark broken in memory sandbox array
                    v = 0 - v;           // Reflect trajectory
                    api->beep(1200, 15);
                    
                    int a = c * 32;
                    int b = r * 15;
                    api->rect(a, b, 30, 12, BLACK); // Directly erase specific shattered block only
                }
            }
        }
        
        // Paddle Bounce Physics Coordinates Check
        if (y > 294) {
            if (x >= p && x <= (p + 60)) {
                y = 294;
                v = 0 - v;
                api->beep(1000, 20);
            }
        }
        
        // Re-render Dynamic Frame Assets (Paddle and Ball elements)
        api->rect(p, q, 60, 6, BLUE);          // Paint Blue Paddle
        api->rect(x - 3, y - 3, 6, 6, YELLOW); // Paint Yellow Ball
        
        if (y > 310) {
            api->beep(150, 600);
            api->clear();
            api->color(RED);
            api->println("\nGAME OVER!");
            api->delay(2000);
            running = 0;
        }
        api->delay(8);
    }
    
    api->flushGameMatrix();        
    api->color(GREEN);
    api->clear();
    return 0;
}
