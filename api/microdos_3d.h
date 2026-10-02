#ifndef MICRODOS_3D_H
#define MICRODOS_3D_H

#include "microdos_api.h"
#include <string.h>

#ifndef M_PI_F
#define M_PI_F 3.14159265f
#endif

struct Vector3D {
    float x;
    float y;
    float z;
};

struct Vector2D {
    int x;
    int y;
};

struct Edge3D {
    uint8_t p1;
    uint8_t p2;
};

ALWAYS INLINE float m3d_reciprocal(float x) {
    if (x == 0.0f) x = 0.001f;

    int32_t i;
    memcpy(&i, &x, sizeof(i));

    i = 0x7EEEEEEE - i;

    float y;
    memcpy(&y, &i, sizeof(y));

    y = y * (2.0f - x * y);
    y = y * (2.0f - x * y);
    return y;
}

ALWAYS INLINE float m3d_sinf(float x) {
    while (x > M_PI_F)  x -= (2.0f * M_PI_F);
    while (x < -M_PI_F) x += (2.0f * M_PI_F);

    float sinVal = 0.0f;
    if (x < 0.0f) {
        sinVal = 1.27323954f * x + 0.405284735f * x * x;
        if (sinVal < 0.0f) sinVal = 0.225f * (sinVal * (-sinVal) - sinVal) + sinVal;
        else               sinVal = 0.225f * (sinVal * sinVal - sinVal) + sinVal;
    } else {
        sinVal = 1.27323954f * x - 0.405284735f * x * x;
        if (sinVal < 0.0f) sinVal = 0.225f * (sinVal * sinVal - sinVal) + sinVal;
        else               sinVal = 0.225f * (sinVal * sinVal - sinVal) + sinVal;
    }
    return sinVal;
}

ALWAYS INLINE float m3d_cosf(float x) {
    return m3d_sinf(x + (M_PI_F * 0.5f));
}

ALWAYS INLINE float m3d_lerp3(float v1, float v2, float v3, float state) {
    if (state <= 1.0f) {
        return v1 + state * (v2 - v1);
    } else {
        float t2 = state - 1.0f;
        return v2 + t2 * (v3 - v2);
    }
}

ALWAYS INLINE void m3d_transform_and_project(
    const Vector3D* inputVertices,
    Vector2D* outputPoints,
    int vertexCount,
    float ax, float ay, float az,
    float morphState,
    const Vector3D* shape2,
    const Vector3D* shape3,
    float scaleFactor, float cameraDistance,
    float screenW, float screenH)
{
    float cx = m3d_cosf(ax), sx = m3d_sinf(ax);
    float cy = m3d_cosf(ay), sy = m3d_sinf(ay);
    float cz = m3d_cosf(az), sz = m3d_sinf(az);

    float halfW = screenW * 0.5f;
    float halfH = screenH * 0.5f;

    for (int i = 0; i < vertexCount; i++) {
        float mx = m3d_lerp3(inputVertices[i].x, shape2[i].x, shape3[i].x, morphState);
        float my = m3d_lerp3(inputVertices[i].y, shape2[i].y, shape3[i].y, morphState);
        float mz = m3d_lerp3(inputVertices[i].z, shape2[i].z, shape3[i].z, morphState);

        float y1 = my * cx - mz * sx;
        float z1 = my * sx + mz * cx;
        float x2 = mx * cy + z1 * sy;
        float z2 = -mx * sy + z1 * cy;
        float x3 = x2 * cz - y1 * sz;
        float y3 = x2 * sz + y1 * cz;

        float invZ = m3d_reciprocal(cameraDistance + z2);

        outputPoints[i].x = (int)((x3 * scaleFactor) * invZ + halfW);
        outputPoints[i].y = (int)((y3 * scaleFactor) * invZ + halfH);
    }
}

#endif // MICRODOS_3D_H
