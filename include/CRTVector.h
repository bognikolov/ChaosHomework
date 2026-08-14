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

// r = d - 2*(d.n)*n
inline CRTVector reflect(const CRTVector& incoming, const CRTVector& normal) {
    return incoming - normal * (2.0f * dot(incoming, normal));
}

// Snell's law. normal points against incoming. Returns false on total internal reflection.
inline bool refract(const CRTVector& incoming, const CRTVector& normal, float eta, CRTVector& outRefracted) {
    float cosThetaI = -dot(incoming, normal);
    float sin2ThetaT = eta * eta * (1.0f - cosThetaI * cosThetaI);
    if (sin2ThetaT > 1.0f) {
        return false;
    }
    float cosThetaT = std::sqrt(1.0f - sin2ThetaT);
    outRefracted = incoming * eta + normal * (eta * cosThetaI - cosThetaT);
    return true;
}

// Schlick's approximation for Fresnel reflectance
inline float fresnelSchlick(float cosThetaI, float iorFrom, float iorTo) {
    float r0 = (iorFrom - iorTo) / (iorFrom + iorTo);
    r0 = r0 * r0;
    float oneMinusCos = 1.0f - cosThetaI;
    return r0 + (1.0f - r0) * (oneMinusCos * oneMinusCos * oneMinusCos * oneMinusCos * oneMinusCos);
}
