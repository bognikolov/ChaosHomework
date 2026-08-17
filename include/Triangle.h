#pragma once
#include "CRTVector.h"

enum class RayType {
    Camera,
    Shadow,
    Reflection,
    Refraction
};

struct Ray {
    CRTVector origin;
    CRTVector direction;
    // Medium the ray is currently traveling through, carried across bounces
    // so refraction knows which side of a boundary it's on.
    float ior = 1.0f;
    RayType type = RayType::Camera;

    Ray(const CRTVector& origin, const CRTVector& direction)
        : origin(origin), direction(direction) {}
    Ray(const CRTVector& origin, const CRTVector& direction, float ior, RayType type)
        : origin(origin), direction(direction), ior(ior), type(type) {}
};

struct HitInfo {
    float t = 0.0f;
    // Barycentric weights w.r.t. (v0, v1, v2).
    float u = 0.0f, v = 0.0f, w = 0.0f;
};

struct CRTTriangle {
    CRTVector v0, v1, v2;
    // Flat face normal, used unless the material wants smooth shading.
    CRTVector normal;
    // Per-vertex normals for smooth shading, filled in by the scene loader.
    CRTVector n0, n1, n2;
    // Per-vertex UVs (u, v, 0), used for checker/bitmap texture sampling.
    CRTVector uv0, uv1, uv2;
    int materialIndex = 0;

    CRTTriangle(const CRTVector& v0, const CRTVector& v1, const CRTVector& v2, int materialIndex = 0)
        : v0(v0), v1(v1), v2(v2), materialIndex(materialIndex) {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        normal = cross(e0, e1).normalize();
        n0 = n1 = n2 = normal;
    }

    float area() const {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        return cross(e0, e1).length() * 0.5f;
    }

    CRTVector smoothNormalAt(float u, float v, float w) const {
        return (n0 * u + n1 * v + n2 * w).normalize();
    }

    // Interpolated (u, v) texture coordinate at the given barycentric weights.
    void uvAt(float u, float v, float w, float& outU, float& outV) const {
        CRTVector uv = uv0 * u + uv1 * v + uv2 * w;
        outU = uv.x;
        outV = uv.y;
    }
};

// Min distance so a bounced ray doesn't immediately re-hit the surface it just left.
static const float T_MIN = 0.001f;

inline bool intersectTriangle(const Ray& ray, const CRTTriangle& tri, HitInfo& hit) {
    float rProj = dot(tri.normal, ray.direction);
    if (std::fabs(rProj) < 1e-6f) {
        return false;
    }

    float rpDist = dot(tri.normal, tri.v0 - ray.origin);
    float t = rpDist / rProj;
    if (t < T_MIN) {
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

    // d1/d2/d0 are the sub-triangle areas opposite v0/v1/v2, so dividing by
    // the total gives the barycentric weights.
    float areaTotal = d0 + d1 + d2;
    if (std::fabs(areaTotal) < 1e-12f) {
        return false;
    }
    float invAreaTotal = 1.0f / areaTotal;

    hit.t = t;
    hit.u = d1 * invAreaTotal;
    hit.v = d2 * invAreaTotal;
    hit.w = d0 * invAreaTotal;
    return true;
}
