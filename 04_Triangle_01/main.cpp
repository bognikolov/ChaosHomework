#include <cmath>
#include <iostream>

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

void printVector(const CRTVector& v) {
    std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

int main() {
    CRTVector a1(3.5f, 0.0f, 0.0f);
    CRTVector b1(1.75f, 3.5f, 0.0f);
    std::cout << "Cross 1: ";
    printVector(cross(a1, b1));
    std::cout << "\n";

    CRTVector a2(3.0f, -3.0f, 1.0f);
    CRTVector b2(4.0f, 9.0f, 3.0f);
    CRTVector c2 = cross(a2, b2);
    std::cout << "Cross 2: ";
    printVector(c2);
    std::cout << " Area: " << c2.length() << "\n";

    CRTVector a3(3.0f, -3.0f, 1.0f);
    CRTVector b3(-12.0f, 12.0f, -4.0f);
    CRTVector c3 = cross(a3, b3);
    std::cout << "Cross 3: ";
    printVector(c3);
    std::cout << " Area: " << c3.length() << "\n";

    Triangle t1(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f)
    );
    std::cout << "Triangle 1 normal: ";
    printVector(t1.normal());
    std::cout << " Area: " << t1.area() << "\n";

    Triangle t2(
        CRTVector(0.0f, 0.0f, -1.0f),
        CRTVector(1.0f, 0.0f, 1.0f),
        CRTVector(-1.0f, 0.0f, 1.0f)
    );
    std::cout << "Triangle 2 normal: ";
    printVector(t2.normal());
    std::cout << " Area: " << t2.area() << "\n";

    Triangle t3(
        CRTVector(0.56f, 1.11f, 1.23f),
        CRTVector(0.44f, -2.368f, -0.54f),
        CRTVector(-1.56f, 0.15f, -1.92f)
    );
    std::cout << "Triangle 3 normal: ";
    printVector(t3.normal());
    std::cout << " Area: " << t3.area() << "\n";

    return 0;
}