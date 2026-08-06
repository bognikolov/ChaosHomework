#pragma once
#include "CRTVector.h"
#include "CRTMatrix.h"
#include "Triangle.h"

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
