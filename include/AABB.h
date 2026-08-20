#pragma once
#include <limits>
#include <algorithm>
#include "CRTVector.h"
#include "Triangle.h"

struct AABB {
    CRTVector min = CRTVector(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    CRTVector max = CRTVector(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    void expand(const CRTVector& p) {
        min.x = std::min(min.x, p.x);
        min.y = std::min(min.y, p.y);
        min.z = std::min(min.z, p.z);
        max.x = std::max(max.x, p.x);
        max.y = std::max(max.y, p.y);
        max.z = std::max(max.z, p.z);
    }

    void expand(const CRTTriangle& tri) {
        expand(tri.v0);
        expand(tri.v1);
        expand(tri.v2);
    }

    void expand(const AABB& other) {
        expand(other.min);
        expand(other.max);
    }

    CRTVector centroid() const {
        return (min + max) * 0.5f;
    }

    // Longest axis: 0=x, 1=y, 2=z. Used to pick the BVH split axis.
    int longestAxis() const {
        CRTVector extent = max - min;
        if (extent.x > extent.y && extent.x > extent.z) return 0;
        if (extent.y > extent.z) return 1;
        return 2;
    }

    // Slab method. tMax caps how far along the ray we care (e.g. distance to a light).
    bool intersect(const Ray& ray, float tMax = std::numeric_limits<float>::max()) const {
        float tMin = 0.0f;

        for (int axis = 0; axis < 3; ++axis) {
            float origin = (axis == 0) ? ray.origin.x : (axis == 1) ? ray.origin.y : ray.origin.z;
            float dir = (axis == 0) ? ray.direction.x : (axis == 1) ? ray.direction.y : ray.direction.z;
            float boxMin = (axis == 0) ? min.x : (axis == 1) ? min.y : min.z;
            float boxMax = (axis == 0) ? max.x : (axis == 1) ? max.y : max.z;

            if (std::fabs(dir) < 1e-12f) {
                if (origin < boxMin || origin > boxMax) {
                    return false;
                }
                continue;
            }

            float invDir = 1.0f / dir;
            float t0 = (boxMin - origin) * invDir;
            float t1 = (boxMax - origin) * invDir;
            if (t0 > t1) std::swap(t0, t1);

            tMin = std::max(tMin, t0);
            tMax = std::min(tMax, t1);
            if (tMin > tMax) {
                return false;
            }
        }
        return true;
    }
};
