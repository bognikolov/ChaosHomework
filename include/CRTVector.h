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

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
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
