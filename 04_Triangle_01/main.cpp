#include <cmath>
#include <iostream>
#include <cassert>

struct CRTVector {
    float x, y, z;

    CRTVector() : x(0), y(0), z(0) {}
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    CRTVector operator-(const CRTVector& other) const {
        return CRTVector(x - other.x, y - other.y, z - other.z);
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
    }
};

CRTVector cross(const CRTVector& a, const CRTVector& b) {
    return CRTVector(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

struct Triangle {
    CRTVector v0, v1, v2;

    Triangle(const CRTVector& v0, const CRTVector& v1, const CRTVector& v2)
        : v0(v0), v1(v1), v2(v2) {}

    CRTVector normal() const {
        CRTVector ab = v1 - v0;
        CRTVector ac = v2 - v0;
        return cross(ab, ac).normalize();
    }

    float area() const {
        CRTVector ab = v1 - v0;
        CRTVector ac = v2 - v0;
        return cross(ab, ac).length() * 0.5f;
    }
};

std::ostream& operator<<(std::ostream& os, const CRTVector& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

bool almostEqual(float a, float b, float epsilon = 1e-4f) {
    return std::fabs(a - b) < epsilon;
}

int main() {
    CRTVector a1(3.5f, 0.0f, 0.0f);
    CRTVector b1(1.75f, 3.5f, 0.0f);
    CRTVector c1 = cross(a1, b1);
    assert(almostEqual(c1.x, 0.0f) && almostEqual(c1.y, 0.0f) && almostEqual(c1.z, 12.25f));
    std::cout << "Cross 1: " << c1 << "\n";

    CRTVector a2(3.0f, -3.0f, 1.0f);
    CRTVector b2(4.0f, 9.0f, 3.0f);
    CRTVector c2 = cross(a2, b2);
    assert(almostEqual(c2.x, -18.0f) && almostEqual(c2.y, -5.0f) && almostEqual(c2.z, 39.0f));
    assert(almostEqual(c2.length(), 43.2435f, 1e-3f));
    std::cout << "Cross 2: " << c2 << " Area: " << c2.length() << "\n";

    CRTVector a3(3.0f, -3.0f, 1.0f);
    CRTVector b3(-12.0f, 12.0f, -4.0f);
    CRTVector c3 = cross(a3, b3);
    assert(almostEqual(c3.x, 0.0f) && almostEqual(c3.y, 0.0f) && almostEqual(c3.z, 0.0f));
    assert(almostEqual(c3.length(), 0.0f));
    std::cout << "Cross 3: " << c3 << " Area: " << c3.length() << "\n";

    Triangle t1(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f)
    );
    CRTVector n1 = t1.normal();
    assert(almostEqual(n1.x, 0.0f) && almostEqual(n1.y, 0.0f) && almostEqual(n1.z, 1.0f));
    assert(almostEqual(t1.area(), 6.125f, 1e-3f));
    std::cout << "Triangle 1 normal: " << n1 << " Area: " << t1.area() << "\n";

    Triangle t2(
        CRTVector(0.0f, 0.0f, -1.0f),
        CRTVector(1.0f, 0.0f, 1.0f),
        CRTVector(-1.0f, 0.0f, 1.0f)
    );
    CRTVector n2 = t2.normal();
    assert(almostEqual(n2.x, 0.0f) && almostEqual(n2.y, -1.0f) && almostEqual(n2.z, 0.0f));
    assert(almostEqual(t2.area(), 2.0f, 1e-3f));
    std::cout << "Triangle 2 normal: " << n2 << " Area: " << t2.area() << "\n";

    Triangle t3(
        CRTVector(0.56f, 1.11f, 1.23f),
        CRTVector(0.44f, -2.368f, -0.54f),
        CRTVector(-1.56f, 0.15f, -1.92f)
    );
    CRTVector n3 = t3.normal();
    assert(almostEqual(n3.x, 0.756420f, 1e-3f) && almostEqual(n3.y, 0.275748f, 1e-3f) && almostEqual(n3.z, -0.593120f, 1e-3f));
    std::cout << "Triangle 3 normal: " << n3 << " Area: " << t3.area() << "\n";

    return 0;
}