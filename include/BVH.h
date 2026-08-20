#pragma once
#include <vector>
#include <algorithm>
#include <memory>
#include "AABB.h"
#include "Triangle.h"
#include "Material.h"

// Leaves stop splitting once they hold this few triangles or fewer.
static const int BVH_LEAF_SIZE = 4;
static const int BVH_MAX_DEPTH = 32;

struct BVHNode {
    AABB bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    // Only meaningful on leaves: range [start, end) into the BVH's triangle index array.
    int start = 0;
    int end = 0;

    bool isLeaf() const { return !left && !right; }
};

class BVH {
public:
    void build(const std::vector<CRTTriangle>& triangles) {
        triangleRefs.resize(triangles.size());
        for (size_t i = 0; i < triangles.size(); ++i) {
            triangleRefs[i] = &triangles[i];
        }
        root = buildRecursive(0, static_cast<int>(triangleRefs.size()), 0);
    }

    bool intersect(const Ray& ray, bool skipRefractive, const std::vector<Material>& materials,
                   float& bestT, const CRTTriangle*& bestTri, HitInfo& bestHit) const {
        if (!root) return false;
        return intersectNode(root.get(), ray, skipRefractive, materials, bestT, bestTri, bestHit);
    }

    const AABB& sceneBounds() const {
        static AABB empty;
        return root ? root->bounds : empty;
    }

private:
    std::vector<const CRTTriangle*> triangleRefs;
    std::unique_ptr<BVHNode> root;

    AABB boundsOf(int start, int end) const {
        AABB bounds;
        for (int i = start; i < end; ++i) {
            bounds.expand(*triangleRefs[i]);
        }
        return bounds;
    }

    std::unique_ptr<BVHNode> buildRecursive(int start, int end, int depth) {
        auto node = std::make_unique<BVHNode>();
        node->bounds = boundsOf(start, end);
        node->start = start;
        node->end = end;

        int count = end - start;
        if (count <= BVH_LEAF_SIZE || depth >= BVH_MAX_DEPTH) {
            return node;
        }

        int axis = node->bounds.longestAxis();
        int mid = start + count / 2;
        std::nth_element(triangleRefs.begin() + start, triangleRefs.begin() + mid, triangleRefs.begin() + end,
                          [axis](const CRTTriangle* a, const CRTTriangle* b) {
                              CRTVector ca = (a->v0 + a->v1 + a->v2) * (1.0f / 3.0f);
                              CRTVector cb = (b->v0 + b->v1 + b->v2) * (1.0f / 3.0f);
                              float va = (axis == 0) ? ca.x : (axis == 1) ? ca.y : ca.z;
                              float vb = (axis == 0) ? cb.x : (axis == 1) ? cb.y : cb.z;
                              return va < vb;
                          });

        node->left = buildRecursive(start, mid, depth + 1);
        node->right = buildRecursive(mid, end, depth + 1);
        return node;
    }

    bool intersectNode(const BVHNode* node, const Ray& ray, bool skipRefractive,
                        const std::vector<Material>& materials,
                        float& bestT, const CRTTriangle*& bestTri, HitInfo& bestHit) const {
        if (!node->bounds.intersect(ray, bestT)) {
            return false;
        }

        if (node->isLeaf()) {
            bool hitAny = false;
            for (int i = node->start; i < node->end; ++i) {
                const CRTTriangle* tri = triangleRefs[i];
                if (skipRefractive && tri->materialIndex >= 0 &&
                    static_cast<size_t>(tri->materialIndex) < materials.size() &&
                    materials[tri->materialIndex].type == MaterialType::Refractive) {
                    continue;
                }
                HitInfo hit;
                if (intersectTriangle(ray, *tri, hit)) {
                    if (hit.t < bestT) {
                        bestT = hit.t;
                        bestTri = tri;
                        bestHit = hit;
                        hitAny = true;
                    }
                }
            }
            return hitAny;
        }

        bool hitLeft = node->left && intersectNode(node->left.get(), ray, skipRefractive, materials, bestT, bestTri, bestHit);
        bool hitRight = node->right && intersectNode(node->right.get(), ray, skipRefractive, materials, bestT, bestTri, bestHit);
        return hitLeft || hitRight;
    }
};
