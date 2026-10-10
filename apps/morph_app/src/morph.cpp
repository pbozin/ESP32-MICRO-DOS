#include "microdos_api.h"
#include "microdos_3d.h"

// --- Shape 1: A standard Unit Cube ---
static const Vector3D cubeLayout[8] __attribute__((aligned(4))) = {
    {-1.0f, -1.0f, -1.0f},
    { 1.0f, -1.0f, -1.0f},
    { 1.0f,  1.0f, -1.0f},
    {-1.0f,  1.0f, -1.0f},
    {-1.0f, -1.0f,  1.0f},
    { 1.0f, -1.0f,  1.0f},
    { 1.0f,  1.0f,  1.0f},
    {-1.0f,  1.0f,  1.0f}
};

// --- Shape 2: An Octahedron (Collapsed Cube) ---
static const Vector3D octaLayout[8] __attribute__((aligned(4))) = {
    { 0.0f,  0.0f, -1.414f}, { 1.414f, 0.0f,  0.0f}, { 0.0f,  0.0f,  1.414f}, {-1.414f, 0.0f,  0.0f},
    { 0.0f,  0.0f, -1.414f}, { 0.0f, -1.414f, 0.0f}, { 0.0f,  0.0f,  1.414f}, { 0.0f,  1.414f, 0.0f}
};

// --- Shape 3: A Tetrahedron (Cube collapsed into 4 nodes) ---
static const Vector3D tetraLayout[8] __attribute__((aligned(4))) = {
    {-1.2f, -1.2f, -1.2f}, // (Node A)
    { 1.2f,  1.2f, -1.2f}, // (Node B)
    { 1.2f,  1.2f, -1.2f}, // (Node B - Collapsed)
    {-1.2f, -1.2f, -1.2f}, // (Node A - Collapsed)
    { 1.2f, -1.2f,  1.2f}, // (Node C)
    { 1.2f, -1.2f,  1.2f}, // (Node C - Collapsed)
    {-1.2f,  1.2f,  1.2f}, // (Node D)
    {-1.2f,  1.2f,  1.2f}  // (Node D - Collapsed)
};

static const Edge3D wireframeTopology[12] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;
    if (!initGameMatrix()) return -1;

    termClear();
    setFKeys(STRING("ZOOM-"), STRING("ZOOM+"), STRING("      "), STRING("      "), STRING(" QUIT "));

    float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
    float currentGlobalState = 0.0f;
    bool cyclingForward = true;

    float screenWidth = (float)getTermWidth();
    float screenHeight = (float)getTermHeight();
    float minDimension = (screenWidth < screenHeight) ? screenWidth : screenHeight;
    
    float scaleFactor = minDimension * 0.25f;
    float cameraDistance = 3.5f;

    Vector2D displayCoordinates[8];
    bool running = true;

    while (running) {
        int key = getKey();
        if (key == '\x15' || key == 'Q' || key == 'q') break;
        
        if (key == '\x11') scaleFactor -= 10.0f;
        if (key == '\x12') scaleFactor += 10.0f;

        if (cyclingForward) {
            currentGlobalState += 0.01f;
            if (currentGlobalState >= 2.0f) {
                currentGlobalState = 2.0f; cyclingForward = false; delayMs(800);
            }
        } else {
            currentGlobalState -= 0.01f;
            if (currentGlobalState <= 0.0f) {
                currentGlobalState = 0.0f; cyclingForward = true; delayMs(800);
            }
        }

        m3d_transform_and_project(
            cubeLayout, displayCoordinates, 8,
            angleX, angleY, angleZ,
            currentGlobalState, 
            octaLayout, tetraLayout,
            scaleFactor, cameraDistance,
            screenWidth, screenHeight
        );

        fillRect(0, 0, getTermWidth(), getTermHeight(), BLACK);

        int strokeColor = CYAN;
        if (currentGlobalState > 1.0f)     strokeColor = GREEN;
        else if (currentGlobalState > 0.5f) strokeColor = MAGENTA;

        for (int i = 0; i < 12; i++) {
            uint8_t a = wireframeTopology[i].p1;
            uint8_t b = wireframeTopology[i].p2;
            drawLine(displayCoordinates[a].x, displayCoordinates[a].y,
                      displayCoordinates[b].x, displayCoordinates[b].y,
                      strokeColor);
        }

        flushGameMatrix();
        
        angleX += 0.02f;
        angleY += 0.03f;
        angleZ += 0.01f;
        
        delayMs(20);
    }

    clearFKeys();
    closeGameMatrix();
    return 0;
}
