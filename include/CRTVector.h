#pragma once
#include <cmath>
#include <iostream>

struct CRTVector {
    float x, y, z;

    CRTVector() : x(0), y(0), z(0) {}
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    CRTVector operator-(const CRTVector& other) const {
        return CRTVector(x - other.x, y - other.y, z - other.z);
    }

    CRTVector operator+(const CRTVector& other) const {
        return CRTVector(x + other.x, y + other.y, z + other.z);
    }

    CRTVector operator*(float s) const {
        return CRTVector(x * s, y * s, z * s);
    }

    CRTVector operator*(const CRTVector& other) const {
        return CRTVector(x * other.x, y * other.y, z * other.z);
    }

    CRTVector operator-() const {
        return CRTVector(-x, -y, -z);
    }

    CRTVector& operator+=(const CRTVector& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    CRTVector normalize() const {
        float len = length();
        if (len < 1e-8f) {
            // Degenerate (zero-length) input has no defined direction; returning
            // a zero vector is safer than dividing by ~0 and propagating inf/NaN.
            return CRTVector(0.0f, 0.0f, 0.0f);
        }
        float invLen = 1.0f / len;
        return CRTVector(x * invLen, y * invLen, z * invLen);
    }
};

inline std::ostream& operator<<(std::ostream& os, const CRTVector& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline float dot(const CRTVector& a, const CRTVector& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline CRTVector cross(const CRTVector& a, const CRTVector& b) {
    return CRTVector(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline bool almostEqual(float a, float b, float epsilon = 1e-4f) {
    return std::fabs(a - b) < epsilon;
}

// Reflects an incoming direction `incoming` about `normal` (both should point "away" consistently;
// normal is assumed normalized). Standard mirror reflection: r = d - 2*(d.n)*n
inline CRTVector reflect(const CRTVector& incoming, const CRTVector& normal) {
    return incoming - normal * (2.0f * dot(incoming, normal));
}
