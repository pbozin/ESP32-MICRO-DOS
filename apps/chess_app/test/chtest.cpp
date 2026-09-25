// src/chess.cpp
#include "microdos_api.h"

#define TKN_F5 '\x15'

extern "C" __attribute__((section(".text._start"))) int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api; // Sync dynamic allocator tracking pointers
    
    if (!api || !api->clear || !api->setFKeys || !api->createSprite || !api->drawSprite) {
        return -1;
    }

    api->setFKeys("", "", "", "", "QUIT");

    api->initGameMatrix();
    // 1. Initialize screen state and soft keys
    //api->clear();
    //api->println("--- PETROX SPRITE RADAR TEST ---");
    //api->println("Attempting to cache sprite into HEAP...");

    // 2. Load exactly one sprite file handle into fast internal RAM
    // (Ensure 'Chess_plt60.spr' is in the current execution folder on SD card)
    uint32_t testPieceHandle = api->createSprite("Chess_plt60.spr");

    if (testPieceHandle == 0) {
        api->color(2); // Red alert text
        api->println("ERR: FILE NOT FOUND OR ALLOCATION FAILED!");
        api->color(4); // Revert back to system green
        
        // Block until user taps F5 to exit safely
        while (api->inkey() != TKN_F5) { api->delay(50); }
        api->clearFKeys();
        return -1;
    }

    //api->println("SUCCESS! Sprite cached in RAM.");
    //api->println("Tap anywhere inside canvas to render...");

    bool testing = true;
    while (testing) {
        // Intercept F5 exit commands
        int key = api->inkey();
        if (key == TKN_F5 || key == 'Q' || key == 'q') {
            testing = false;
            break;
        }

        TouchState touch;
        api->getTouch(&touch);

        // If user touches inside the 320x320 canvas viewport arena
        if (touch.isPressed && touch.y >= 0 && touch.y < 320 && touch.x >= 0 && touch.x < 320) {
            // Draw a flat baseline background square first (Palette ID 7 = White)
            // This lets us confirm the chroma-key transparency mask is working!
            //api->rect(touch.x - 20, touch.y - 20, 40, 40, 7);

            // Blit our cached RAM sprite right over the center coordinate
            // Magenta Chroma-Key palette ID = 3
            api->drawSprite(testPieceHandle, touch.x, touch.y);
            
            //api->print("Rendered sprite at X: ");
            //char xBuf[16];
            // Simple integer-to-string fallback safely bypassing sprintf
            //int tempX = touch.x;
            //xBuf[0] = '0' + (tempX / 100);
            //xBuf[1] = '0' + ((tempX / 10) % 10);
            //xBuf[2] = '0' + (tempX % 10);
            //xBuf[3] = '\0';
            //api->println(xBuf);

            api->delay(250); // Gentle touch debounce
        }
        api->delay(30);
    }

    // 3. Garbage collection: release RAM block before returning to shell prompt
    //api->println("Cleaning up memory registers...");
    api->freeSprite(testPieceHandle);

    api->clearFKeys();
    api->closeGameMatrix();
    api->delay(500);
    //api->clear();
    return 0;
}
