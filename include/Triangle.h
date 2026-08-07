#pragma once
#include "CRTVector.h"
#include "CRTColor.h"

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
    CRTVector albedo;

    CRTTriangle(const CRTVector& v0, const CRTVector& v1, const CRTVector& v2, const CRTColor& color,
                const CRTVector& albedo = CRTVector(0.7f, 0.7f, 0.7f))
        : v0(v0), v1(v1), v2(v2), color(color), albedo(albedo) {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        normal = cross(e0, e1).normalize();
    }

    float area() const {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        return cross(e0, e1).length() * 0.5f;
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
