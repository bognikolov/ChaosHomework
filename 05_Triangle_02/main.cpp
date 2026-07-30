#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <limits>

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

float dot(const CRTVector& a, const CRTVector& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

CRTVector cross(const CRTVector& a, const CRTVector& b) {
    return CRTVector(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

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

bool intersectTriangle(const Ray& ray, const CRTTriangle& tri, float& tOut) {
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

CRTVector generateRayDirection(int x, int y, int width, int height) {
    float fx = static_cast<float>(x) + 0.5f;
    float fy = static_cast<float>(y) + 0.5f;

    fx /= static_cast<float>(width);
    fy /= static_cast<float>(height);

    fx = (2.0f * fx) - 1.0f;
    fy = 1.0f - (2.0f * fy);

    fx *= static_cast<float>(width) / static_cast<float>(height);

    CRTVector dir(fx, fy, -1.0f);
    return dir.normalize();
}

static const int imageWidth = 1280;
static const int imageHeight = 720;
static const int maxColorComponent = 255;

int main() {
    std::vector<CRTTriangle> scene;

    CRTVector apex(0.0f, 1.2f, -4.0f);
    CRTVector base1(-1.3f, -0.8f, -3.0f);
    CRTVector base2(1.3f, -0.8f, -3.0f);
    CRTVector base3(0.0f, -0.8f, -5.0f);

    scene.push_back(CRTTriangle(apex, base1, base2, CRTColor(220, 60, 60)));
    scene.push_back(CRTTriangle(apex, base2, base3, CRTColor(60, 220, 60)));
    scene.push_back(CRTTriangle(apex, base3, base1, CRTColor(60, 60, 220)));
    scene.push_back(CRTTriangle(base1, base3, base2, CRTColor(220, 220, 60)));

    CRTColor backgroundColor(30, 30, 30);
    CRTVector cameraOrigin(0.0f, 0.0f, 0.0f);

    std::ofstream ppmFileStream("crt_hw05_output.ppm", std::ios::out | std::ios::binary);
    ppmFileStream << "P3\n";
    ppmFileStream << imageWidth << " " << imageHeight << "\n";
    ppmFileStream << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            CRTVector rayDir = generateRayDirection(colIdx, rowIdx, imageWidth, imageHeight);
            Ray ray(cameraOrigin, rayDir);

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
    return 0;
}