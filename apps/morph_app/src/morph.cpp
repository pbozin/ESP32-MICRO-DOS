#include "microdos_api.h"

#define M_PI_F 3.14159265f

ALWAYS INLINE float custom_reciprocal(float x) {
    if (x == 0.0f) x = 0.001f;

    int32_t i = *(int32_t*)&x;
    i = 0x7EEEEEEE - i;
    float y = *(float*)&i;

    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);

    return y;
}

ALWAYS INLINE float custom_sinf(float x) {
    while (x > M_PI_F)  x -= (2.0f * M_PI_F);
    while (x < -M_PI_F) x += (2.0f * M_PI_F);
    
    float sinVal = 0.0f;
    if (x < 0.0f) {
        sinVal = 1.27323954f * x + 0.405284735f * x * x;
        if (sinVal < 0.0f) sinVal = 0.225f * (sinVal * (-sinVal) - sinVal) + sinVal;
        else               sinVal = 0.225f * (sinVal * sinVal - sinVal) + sinVal;
    } else {
        sinVal = 1.27323954f * x - 0.405284735f * x * x;
        if (sinVal < 0.0f) sinVal = 0.225f * (sinVal * (-sinVal) - sinVal) + sinVal;
        else               sinVal = 0.225f * (sinVal * sinVal - sinVal) + sinVal;
    }
    return sinVal;
}

ALWAYS INLINE float custom_cosf(float x) {
    return custom_sinf(x + (M_PI_F * 0.5f));
}

struct Point3D {
    float x;
    float y;
    float z;
};

struct Point2D {
    int x;
    int y;
};

static const Point3D cubeVertices[8] __attribute__((aligned(4))) = {
    {-1.0f, -1.0f, -1.0f}, 
    { 1.0f, -1.0f, -1.0f}, 
    { 1.0f,  1.0f, -1.0f}, 
    {-1.0f,  1.0f, -1.0f}, 
    {-1.0f, -1.0f,  1.0f}, 
    { 1.0f, -1.0f,  1.0f}, 
    { 1.0f,  1.0f,  1.0f}, 
    {-1.0f,  1.0f,  1.0f}  
};

static const Point3D octaVertices[8] __attribute__((aligned(4))) = {
    { 0.0f,  0.0f, -1.414f}, 
    { 1.414f, 0.0f,  0.0f},  
    { 0.0f,  0.0f,  1.414f}, 
    {-1.414f, 0.0f,  0.0f},  
    { 0.0f,  0.0f, -1.414f}, 
    { 0.0f, -1.414f, 0.0f},  
    { 0.0f,  0.0f,  1.414f}, 
    { 0.0f,  1.414f, 0.0f}   
};

static const uint8_t edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

extern "C" int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;
    
    if (!api->initGameMatrix()) return -1;

    api->clear();
    api->setFKeys(STRING("      "), STRING("      "), STRING("      "), STRING("      "), STRING(" QUIT "));

    float angleX = 0.0f;
    float angleY = 0.0f;
    float angleZ = 0.0f;

    float morphTime = 0.0f;
    bool morphDirection = true; 

    float screenWidth = (float)api->termWidth;
    float screenHeight = (float)api->termHeight;
    float minDimension = (screenWidth < screenHeight) ? screenWidth : screenHeight;
    
    float scaleFactor = minDimension * 0.25f; 
    float cameraDistance = 3.5f; 

    Point2D projectedPoints[8];
    bool running = true;

    while (running) {
        int key = api->inkey();
        if (key == '\x15' || key == 'Q' || key == 'q') {
            running = false;
            break;
        }

        if (morphDirection) {
            morphTime += 0.015f;
            if (morphTime >= 1.0f) { morphTime = 1.0f; morphDirection = false; api->delay(500); }
        } else {
            morphTime -= 0.015f;
            if (morphTime <= 0.0f) { morphTime = 0.0f; morphDirection = true; api->delay(500); }
        }

        float t = (custom_sinf((morphTime * M_PI_F) - (M_PI_F * 0.5f)) + 1.0f) * 0.5f;

        float cx = custom_cosf(angleX), sx = custom_sinf(angleX);
        float cy = custom_cosf(angleY), sy = custom_sinf(angleY);
        float cz = custom_cosf(angleZ), sz = custom_sinf(angleZ);

        for (int i = 0; i < 8; i++) {
            float mx = cubeVertices[i].x + t * (octaVertices[i].x - cubeVertices[i].x);
            float my = cubeVertices[i].y + t * (octaVertices[i].y - cubeVertices[i].y);
            float mz = cubeVertices[i].z + t * (octaVertices[i].z - cubeVertices[i].z);

            float y1 = my * cx - mz * sx;
            float z1 = my * sx + mz * cx;

            float x2 = mx * cy + z1 * sy;
            float z2 = -mx * sy + z1 * cy;

            float x3 = x2 * cz - y1 * sz;
            float y3 = x2 * sz + y1 * cz;

            float perspectiveZ = cameraDistance + z2;
            float invZ = custom_reciprocal(perspectiveZ);
            
            projectedPoints[i].x = (int)((x3 * scaleFactor) * invZ + (screenWidth * 0.5f));
            projectedPoints[i].y = (int)((y3 * scaleFactor) * invZ + (screenHeight * 0.5f));
        }

        api->rect(0, 0, api->termWidth, api->termHeight, BLACK);

        int activeColor = (t < 0.5f) ? CYAN : MAGENTA;

        for (int i = 0; i < 12; i++) {
            int p1 = edges[i][0];
            int p2 = edges[i][1];
            api->line(projectedPoints[p1].x, projectedPoints[p1].y, 
                      projectedPoints[p2].x, projectedPoints[p2].y, 
                      activeColor);
        }

        api->flushGameMatrix();
        angleX += 0.02f;
        angleY += 0.03f;
        angleZ += 0.01f;

        api->delay(20); 
    }

    api->clearFKeys();
    api->closeGameMatrix();
    return 0;
}
