#pragma once
#include "CRTVector.h"

static const float PI = 3.14159265358979323846f;

inline float toRadians(float degrees) {
    return degrees * PI / 180.0f;
}

struct CRTMatrix3x3 {
    float m[3][3];

    CRTMatrix3x3() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

private:
    // used internally to skip the identity-fill when we're about to overwrite everything anyway
    struct Uninitialized {};
    explicit CRTMatrix3x3(Uninitialized) {}

public:
    static CRTMatrix3x3 rotationX(float degrees) {
        float rad = toRadians(degrees);
        float c = std::cos(rad);
        float s = std::sin(rad);
        CRTMatrix3x3 mat{Uninitialized{}};
        mat.m[0][0] = 1; mat.m[0][1] = 0; mat.m[0][2] = 0;
        mat.m[1][0] = 0; mat.m[1][1] = c; mat.m[1][2] = -s;
        mat.m[2][0] = 0; mat.m[2][1] = s; mat.m[2][2] = c;
        return mat;
    }

    static CRTMatrix3x3 rotationY(float degrees) {
        float rad = toRadians(degrees);
        float c = std::cos(rad);
        float s = std::sin(rad);
        CRTMatrix3x3 mat{Uninitialized{}};
        mat.m[0][0] = c;  mat.m[0][1] = 0; mat.m[0][2] = s;
        mat.m[1][0] = 0;  mat.m[1][1] = 1; mat.m[1][2] = 0;
        mat.m[2][0] = -s; mat.m[2][1] = 0; mat.m[2][2] = c;
        return mat;
    }

    static CRTMatrix3x3 rotationZ(float degrees) {
        float rad = toRadians(degrees);
        float c = std::cos(rad);
        float s = std::sin(rad);
        CRTMatrix3x3 mat{Uninitialized{}};
        mat.m[0][0] = c; mat.m[0][1] = -s; mat.m[0][2] = 0;
        mat.m[1][0] = s; mat.m[1][1] = c;  mat.m[1][2] = 0;
        mat.m[2][0] = 0; mat.m[2][1] = 0;  mat.m[2][2] = 1;
        return mat;
    }

    CRTVector operator*(const CRTVector& v) const {
        return CRTVector(
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        );
    }

    CRTMatrix3x3 operator*(const CRTMatrix3x3& other) const {
        CRTMatrix3x3 result{Uninitialized{}};
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 3; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
};
