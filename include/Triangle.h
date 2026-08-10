#pragma once
#include "CRTVector.h"

struct Ray {
    CRTVector origin;
    CRTVector direction;
    Ray(const CRTVector& origin, const CRTVector& direction)
        : origin(origin), direction(direction) {}
};

// Result of a ray-triangle intersection, including barycentric coordinates
// so callers can interpolate normals, colors, UVs, etc.
struct HitInfo {
    float t = 0.0f;
    // Barycentric weights w.r.t. (v0, v1, v2). u+v+w == 1.
    float u = 0.0f, v = 0.0f, w = 0.0f;
};

struct CRTTriangle {
    CRTVector v0, v1, v2;
    // Flat (geometric) face normal - always available, used when smooth shading is off.
    CRTVector normal;
    // Optional per-vertex normals - used for smooth (Phong) shading when the
    // triangle's material has smooth_shading = true. Populated by the scene loader.
    CRTVector n0, n1, n2;
    int materialIndex = 0;

    CRTTriangle(const CRTVector& v0, const CRTVector& v1, const CRTVector& v2, int materialIndex = 0)
        : v0(v0), v1(v1), v2(v2), materialIndex(materialIndex) {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        normal = cross(e0, e1).normalize();
        // Default vertex normals to the flat normal until a smooth loader overrides them.
        n0 = n1 = n2 = normal;
    }

    float area() const {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        return cross(e0, e1).length() * 0.5f;
    }

    // Interpolated normal at the given barycentric weights, using vertex normals.
    CRTVector smoothNormalAt(float u, float v, float w) const {
        return (n0 * u + n1 * v + n2 * w).normalize();
    }
};

inline bool intersectTriangle(const Ray& ray, const CRTTriangle& tri, HitInfo& hit) {
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

    // Barycentric weights: d1/area correspond to vertex v0's opposite sub-triangle, etc.
    // With e0=(v1-v0), e1=(v2-v1), e2=(v0-v2), the sub-triangle areas (d1, d2, d0)
    // are proportional to the barycentric weights of (v0, v1, v2) respectively.
    float areaTotal = d0 + d1 + d2;
    if (std::fabs(areaTotal) < 1e-12f) {
        return false;
    }
    float invAreaTotal = 1.0f / areaTotal;

    hit.t = t;
    hit.u = d1 * invAreaTotal; // weight for v0
    hit.v = d2 * invAreaTotal; // weight for v1
    hit.w = d0 * invAreaTotal; // weight for v2
    return true;
}

// Backward-compatible overload returning just the distance along the ray.
inline bool intersectTriangle(const Ray& ray, const CRTTriangle& tri, float& tOut) {
    HitInfo hit;
    if (!intersectTriangle(ray, tri, hit)) {
        return false;
    }
    tOut = hit.t;
    return true;
}
