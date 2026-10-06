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

struct AppDataBuffer {
    Vector3D sphereVertices[TOTAL_VERTICES];
    Edge sphereEdges[MAX_EDGES];
    Vector2D projectedPoints[TOTAL_VERTICES];
    int edgeCount;
};

static struct AppDataBuffer db __attribute__((aligned(4)));

ALWAYS INLINE void generateProceduralSphere(float radius) {
    int vIdx = 0;
    int localEdgeCount = 0;
    db.sphereVertices[vIdx++] = { 0.0f, radius, 0.0f };

    for (int lat = 1; lat < LATITUDE_BANDS; lat++) {
        float theta = ((float)lat * M_PI_F) * INV_LAT_BANDS;
        float sinTheta = m3d_sinf(theta);
        float cosTheta = m3d_cosf(theta);

        for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
            float phi = ((float)lon * 2.0f * M_PI_F) * INV_LON_BANDS;

            float x = radius * sinTheta * m3d_cosf(phi);
            float y = radius * cosTheta;
            float z = radius * sinTheta * m3d_sinf(phi);

            db.sphereVertices[vIdx++] = { x, y, z };
        }
    }

    int bottomPoleIdx = vIdx;
    db.sphereVertices[vIdx++] = { 0.0f, -radius, 0.0f };

    // Connect Top Pole
    for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
        db.sphereEdges[localEdgeCount++] = { 0, (uint16_t)(1 + lon) };
    }

    // Connect intermediate rings
    for (int lat = 0; lat < LATITUDE_BANDS - 2; lat++) {
        int ringStart = 1 + lat * LONGITUDE_BANDS;
        int nextRingStart = ringStart + LONGITUDE_BANDS;

        for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
            int nextLon = (lon + 1) % LONGITUDE_BANDS;
            db.sphereEdges[localEdgeCount++] = { (uint16_t)(ringStart + lon), (uint16_t)(nextRingStart + lon) };
            db.sphereEdges[localEdgeCount++] = { (uint16_t)(ringStart + lon), (uint16_t)(ringStart + nextLon) };
        }
    }

    // Connect Bottom Pole (Fixed tracking index variables)
    int finalRingStart = 1 + (LATITUDE_BANDS - 2) * LONGITUDE_BANDS;
    for (int lon = 0; lon < LONGITUDE_BANDS; lon++) {
        int nextLon = (lon + 1) % LONGITUDE_BANDS;
        db.sphereEdges[localEdgeCount++] = { (uint16_t)(finalRingStart + lon), (uint16_t)bottomPoleIdx };
        db.sphereEdges[localEdgeCount++] = { (uint16_t)(finalRingStart + lon), (uint16_t)(finalRingStart + nextLon) };
    }

    db.edgeCount = localEdgeCount;
}

extern "C" int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api;
    if (!api) return -1;
    if (!initGameMatrix()) return -1;

    setFKeys(STRING(" ZOOM-"), STRING(" ZOOM+"), STRING("      "), STRING("      "), STRING(" QUIT "));

    float screenWidth  = (float)getTermWidth();
    float screenHeight = (float)getTermHeight();

    float radiusLimit  = screenWidth * 0.25f;

    float ballX        = screenWidth * 0.5f;
    float ballY        = screenHeight * 0.33f;

    float velX         = 2.5f;
    float velY         = 0.0f;
    float gravity      = 0.18f;
    float bounceLoss   = -0.85f;

    float angleX       = 0.0f;
    float angleY       = 0.0f;
    float spinSpeedY   = 0.04f;

    generateProceduralSphere(1.0f);

    bool running = true;

    while (running) {
        int key = getKey();
        if (key == '\x15' || key == 'Q' || key == 'q') break;

        if (key == '\x11') {
            radiusLimit -= 5.0f;
            if (radiusLimit < 10.0f) radiusLimit = 10.0f;
        }
        if (key == '\x12') {
            radiusLimit += 5.0f;
            if (radiusLimit > (screenWidth * 0.75f)) radiusLimit = screenWidth * 0.75f;
        }

        velY += gravity;
        ballX += velX;
        ballY += velY;

        float realRadius = radiusLimit * 0.45f;

        if (ballX - realRadius < 0.0f) {
            ballX = realRadius;
            velX = -velX;
        }
        if (ballX + realRadius > screenWidth) {
            ballX = screenWidth - realRadius;
            velX = -velX;
        }

        if (ballY + realRadius > screenHeight) {
            ballY = screenHeight - realRadius;
            velY *= bounceLoss;
            if (m3d_reciprocal(velY) > 5.0f && velY > -0.5f) velY = -3.5f;
        }

        angleY += spinSpeedY;
        angleX += 0.01f;

        m3d_transform_and_project(
            db.sphereVertices, db.projectedPoints, TOTAL_VERTICES,
            angleX, angleY, 0.0f,
            0.0f, db.sphereVertices, db.sphereVertices,
            radiusLimit, 2.5f,
            screenWidth, screenHeight
        );

        drawRect(0, 0, getTermWidth(), getTermHeight(), BLACK);

        int offsetX = (int)ballX - (int)(screenWidth * 0.5f);
        int offsetY = (int)ballY - (int)(screenHeight * 0.5f);

        for (int i = 0; i < db.edgeCount; i++) {
            uint16_t p1 = db.sphereEdges[i].p1;
            uint16_t p2 = db.sphereEdges[i].p2;
            int strokeColor = (p1 % 2 == 0) ? RED : WHITE;

            drawLine(db.projectedPoints[p1].x + offsetX, db.projectedPoints[p1].y + offsetY,
                      db.projectedPoints[p2].x + offsetX, db.projectedPoints[p2].y + offsetY,
                      strokeColor);
        }

        flushGameMatrix();
        delayMs(16);
    }

    clearFKeys();
    closeGameMatrix();
    return 0;
}
