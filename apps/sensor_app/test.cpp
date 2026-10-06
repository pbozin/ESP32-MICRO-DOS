#include "microdos_api.h" 

// Pin definitions
#define LED_RED   22
#define LED_GREEN 16
#define LED_BLUE  17

// Common Anode Logic (Low = ON, High = OFF)
#define LED_ON  0
#define LED_OFF 1

static MicroDosAPI* os ALIGNED = NULL;

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    os = api;
    // Configure the RGB pins as outputs
    os->pinMode(LED_RED, OUTPUT);
    os->pinMode(LED_GREEN, OUTPUT);
    os->pinMode(LED_BLUE, OUTPUT);

    // Clear all LEDs to OFF at startup
    os->digitalWrite(LED_RED, LED_OFF);
    os->digitalWrite(LED_GREEN, LED_OFF);
    os->digitalWrite(LED_BLUE, LED_OFF);

    // Test loop: Cycles Red -> Green -> Blue
    while(1) {
        // Red Flash
        os->digitalWrite(LED_RED, LED_ON);
        os->delay(500);
        os->digitalWrite(LED_RED, LED_OFF);

        // Green Flash
        os->digitalWrite(LED_GREEN, LED_ON);
        os->delay(500);
        os->digitalWrite(LED_GREEN, LED_OFF);

        // Blue Flash
        os->digitalWrite(LED_BLUE, LED_ON);
        os->delay(500);
        os->digitalWrite(LED_BLUE, LED_OFF);
        
        // Brief pause before restarting cycle
        os->delay(500);
    }
}
