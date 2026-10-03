#include "microdos_api.h"
#include "microdos_3d.h"

#define LATITUDE_BANDS  8
#define LONGITUDE_BANDS 12
#define TOTAL_VERTICES  ((LATITUDE_BANDS - 1) * LONGITUDE_BANDS + 2)
#define MAX_EDGES       (LATITUDE_BANDS * LONGITUDE_BANDS * 2)

#define INV_LAT_BANDS   (1.0f / (float)LATITUDE_BANDS)
#define INV_LON_BANDS   (1.0f / (float)LONGITUDE_BANDS)

struct Edge {
    uint16_t p1;
    uint16_t p2;
};

static Vector3D sphereVertices[TOTAL_VERTICES] ALIGNED;
static Edge sphereEdges[MAX_EDGES] ALIGNED;
static Vector2D projectedPoints[TOTAL_VERTICES] ALIGNED;
static int edgeCount ALIGNED = 0;

ALWAYS INLINE void generateProceduralSphere(float radius, MicroDosAPI* api) {
    int vIdx = 0;
    int localEdgeCount = 0;
    sphereVertices[vIdx++] = { 0.0f, radius, 0.0f };

    for (int lat = 1; lat < LATITUDE_BANDS; lat++) {
        float theta ALIGNED = ((float)lat * M_PI_F) * INV_LAT_BANDS;
        float sinTheta ALIGNED = m3d_sinf(theta);
        float cosTheta ALIGNED = m3d_cosf(theta);

        for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
            float phi ALIGNED = ((float)lon * 2.0f * M_PI_F) * INV_LON_BANDS;
            
            float x ALIGNED = radius * sinTheta * m3d_cosf(phi);
            float y ALIGNED = radius * cosTheta;
            float z ALIGNED = radius * sinTheta * m3d_sinf(phi);

            sphereVertices[vIdx++] = { x, y, z };
        }
    }

    int bottomPoleIdx = vIdx;
    sphereVertices[vIdx++] = { 0.0f, -radius, 0.0f };

    // Connect Top Pole
    for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
        sphereEdges[localEdgeCount++] = { 0, (uint16_t)(1 + lon) };
    }

    // Connect intermediate rings
    for (int lat = 0; lat < LATITUDE_BANDS - 2; lat++) {
        int ringStart = 1 + lat * LONGITUDE_BANDS;
        int nextRingStart = ringStart + LONGITUDE_BANDS;

        for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
            int nextLon = (lon + 1) % LONGITUDE_BANDS;
            sphereEdges[localEdgeCount++] = { (uint16_t)(ringStart + lon), (uint16_t)(nextRingStart + lon) };
            sphereEdges[localEdgeCount++] = { (uint16_t)(ringStart + lon), (uint16_t)(ringStart + nextLon) };
        }
    }

    // Connect Bottom Pole
    int finalRingStart = 1 + (LATITUDE_BANDS - 2) * LONGITUDE_BANDS;
    for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
        int nextLon = (lon + 1) % LONGITUDE_BANDS;
        sphereEdges[localEdgeCount++] = { (uint16_t)(finalRingStart + lon), (uint16_t)bottomPoleIdx };
        sphereEdges[localEdgeCount++] = { (uint16_t)(finalRingStart + lon), (uint16_t)(finalRingStart + nextLon) };
    }

    edgeCount = localEdgeCount;
}

int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api || !api->initGameMatrix()) return -1;

    api->setFKeys(STRING(" ZOOM-"), STRING(" ZOOM+"), STRING("      "), STRING("      "), STRING(" QUIT "));

    float screenWidth  ALIGNED = (float)api->termWidth;
    float screenHeight ALIGNED = (float)api->termHeight;
    float ballX        ALIGNED = screenWidth * 0.5f;
    float ballY        ALIGNED = screenHeight * 0.33f;

    float velX         ALIGNED = 2.5f;
    float velY         ALIGNED = 0.0f;
    float gravity      ALIGNED = 0.18f;
    float bounceLoss   ALIGNED = -0.85f;

    float angleX       ALIGNED = 0.0f;
    float angleY       ALIGNED = 0.0f;
    float spinSpeedY   ALIGNED = 0.04f;
    float radiusLimit  ALIGNED = screenWidth * 0.12f;

    generateProceduralSphere(1.0f, api);

    kernelDebug(STRING("SPHERE VERTICES BASE"), sphereVertices, TOTAL_VERTICES, (uint32_t)sphereVertices);
    kernelDebug(STRING("PROJECTED BUFFER BASE"), projectedPoints, TOTAL_VERTICES, (uint32_t)projectedPoints);

    bool running ALIGNED = true;

    while (running) {
        int key = api->inkey();

        if (key == '\x15' || key == 'Q' || key == 'q') break;

        if (key == '\x11') { 
            radiusLimit -= 5.0f; 
        }
        if (key == '\x12') { 
            radiusLimit += 5.0f; 
        }

        velY += gravity;
        ballX += velX;
        ballY += velY;

        if (ballX - radiusLimit < 0.0f) { ballX = radiusLimit; velX = -velX; }
        if (ballX + radiusLimit > screenWidth) { ballX = screenWidth - radiusLimit; velX = -velX; }
        
        if (ballY + radiusLimit > screenHeight) {
            ballY = screenHeight - radiusLimit;
            velY *= bounceLoss;
            if (m3d_reciprocal(velY) > 5.0f && velY > -0.5f) velY = -3.5f; 
        }

        angleY += spinSpeedY;
        angleX += 0.01f; 

        m3d_transform_and_project(
            sphereVertices, projectedPoints, TOTAL_VERTICES,
            angleX, angleY, 0.0f,
            0.0f, sphereVertices, sphereVertices, 
            radiusLimit, 2.5f,
            screenWidth, screenHeight
        );

        api->rect(0, 0, api->termWidth, api->termHeight, BLACK);

        int offsetX = (int)ballX - (int)(screenWidth * 0.5f);
        int offsetY = (int)ballY - (int)(screenHeight * 0.5f);

        for (int i = 0; i < edgeCount; i++) {
            uint16_t p1 = sphereEdges[i].p1;
            uint16_t p2 = sphereEdges[i].p2;
            int strokeColor = (p1 % 2 == 0) ? RED : WHITE;

            api->line(projectedPoints[p1].x + offsetX, projectedPoints[p1].y + offsetY,
                      projectedPoints[p2].x + offsetX, projectedPoints[p2].y + offsetY,
                      strokeColor);
        }

        api->flushGameMatrix();
        api->delay(16); 
    }

    api->clearFKeys();
    api->closeGameMatrix();
    return 0;
}
