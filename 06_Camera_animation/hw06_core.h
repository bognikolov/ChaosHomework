#pragma once
#include <fstream>
#include <cmath>
#include <vector>
#include <limits>
#include <iostream>
#include <string>
#include <iomanip>

static const float PI = 3.14159265358979323846f;

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

    static CRTMatrix3x3 rotationX(float degrees) {
        float rad = toRadians(degrees);
        float c = std::cos(rad);
        float s = std::sin(rad);
        CRTMatrix3x3 mat;
        mat.m[0][0] = 1; mat.m[0][1] = 0; mat.m[0][2] = 0;
        mat.m[1][0] = 0; mat.m[1][1] = c; mat.m[1][2] = -s;
        mat.m[2][0] = 0; mat.m[2][1] = s; mat.m[2][2] = c;
        return mat;
    }

    static CRTMatrix3x3 rotationY(float degrees) {
        float rad = toRadians(degrees);
        float c = std::cos(rad);
        float s = std::sin(rad);
        CRTMatrix3x3 mat;
        mat.m[0][0] = c;  mat.m[0][1] = 0; mat.m[0][2] = s;
        mat.m[1][0] = 0;  mat.m[1][1] = 1; mat.m[1][2] = 0;
        mat.m[2][0] = -s; mat.m[2][1] = 0; mat.m[2][2] = c;
        return mat;
    }

    static CRTMatrix3x3 rotationZ(float degrees) {
        float rad = toRadians(degrees);
        float c = std::cos(rad);
        float s = std::sin(rad);
        CRTMatrix3x3 mat;
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
        CRTMatrix3x3 result;
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

struct CRTColor {
    int r, g, b;
    CRTColor() : r(0), g(0), b(0) {}
    CRTColor(int r, int g, int b) : r(r), g(g), b(b) {}
};

struct Ray {
    CRTVector origin;
    CRTVector direction;
    Ray(const CRTVector& origin, const CRTVector& direction)
        : origin(origin), direction(direction) {}
};

struct CRTTriangle {
    CRTVector v0, v1, v2;
    CRTVector normal;
    CRTColor color;

    CRTTriangle(const CRTVector& v0, const CRTVector& v1, const CRTVector& v2, const CRTColor& color)
        : v0(v0), v1(v1), v2(v2), color(color) {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        normal = cross(e0, e1).normalize();
    }
};

inline bool intersectTriangle(const Ray& ray, const CRTTriangle& tri, float& tOut) {
    float rProj = dot(tri.normal, ray.direction);
    if (std::fabs(rProj) < 1e-6f) {
        return false;
    }

    float rpDist = dot(tri.normal, tri.v0 - ray.origin);
    float t = rpDist / rProj;
    if (t < 0.0f) {
        return false;
    }

    CRTVector p = ray.origin + ray.direction * t;

    CRTVector e0 = tri.v1 - tri.v0;
    CRTVector v0p = p - tri.v0;
    float d0 = dot(tri.normal, cross(e0, v0p));

    CRTVector e1 = tri.v2 - tri.v1;
    CRTVector v1p = p - tri.v1;
    float d1 = dot(tri.normal, cross(e1, v1p));

    CRTVector e2 = tri.v0 - tri.v2;
    CRTVector v2p = p - tri.v2;
    float d2 = dot(tri.normal, cross(e2, v2p));

    bool inside = (d0 >= 0.0f && d1 >= 0.0f && d2 >= 0.0f) ||
                  (d0 <= 0.0f && d1 <= 0.0f && d2 <= 0.0f);

    if (!inside) {
        return false;
    }

    tOut = t;
    return true;
}

struct Camera {
    CRTVector position;
    CRTMatrix3x3 rotation;

    Camera() : position(0.0f, 0.0f, 0.0f) {}
    Camera(const CRTVector& position) : position(position) {}

    CRTVector forward() const { return rotation * CRTVector(0.0f, 0.0f, -1.0f); }
    CRTVector right() const { return rotation * CRTVector(1.0f, 0.0f, 0.0f); }
    CRTVector up() const { return rotation * CRTVector(0.0f, 1.0f, 0.0f); }

    void pan(float degrees) {
        rotation = CRTMatrix3x3::rotationY(degrees) * rotation;
    }

    void tilt(float degrees) {
        rotation = CRTMatrix3x3::rotationX(degrees) * rotation;
    }

    void roll(float degrees) {
        rotation = CRTMatrix3x3::rotationZ(degrees) * rotation;
    }

    void truck(float distance) {
        position = position + right() * distance;
    }

    void pedestal(float distance) {
        position = position + up() * distance;
    }

    void dolly(float distance) {
        position = position + forward() * distance;
    }

    Ray generateRay(int x, int y, int width, int height) const {
        float fx = static_cast<float>(x) + 0.5f;
        float fy = static_cast<float>(y) + 0.5f;

        fx /= static_cast<float>(width);
        fy /= static_cast<float>(height);

        fx = (2.0f * fx) - 1.0f;
        fy = 1.0f - (2.0f * fy);

        fx *= static_cast<float>(width) / static_cast<float>(height);

        CRTVector localDir(fx, fy, -1.0f);
        CRTVector worldDir = (rotation * localDir).normalize();
        return Ray(position, worldDir);
    }
};

inline void renderScene(const Camera& camera, const std::vector<CRTTriangle>& scene,
                         int width, int height, const std::string& filename) {
    CRTColor backgroundColor(30, 30, 30);

    std::ofstream ppmFileStream(filename, std::ios::out | std::ios::binary);
    ppmFileStream << "P3\n" << width << " " << height << "\n255\n";

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            Ray ray = camera.generateRay(colIdx, rowIdx, width, height);

            float closestT = std::numeric_limits<float>::max();
            const CRTTriangle* closestTri = nullptr;

            for (const CRTTriangle& tri : scene) {
                float t;
                if (intersectTriangle(ray, tri, t)) {
                    if (t < closestT) {
                        closestT = t;
                        closestTri = &tri;
                    }
                }
            }

            CRTColor pixelColor = closestTri ? closestTri->color : backgroundColor;
            ppmFileStream << pixelColor.r << " " << pixelColor.g << " " << pixelColor.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}
