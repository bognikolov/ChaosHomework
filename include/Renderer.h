#pragma once
#include <fstream>
#include <vector>
#include <limits>
#include <string>
#include <algorithm>
#include "Camera.h"
#include "Triangle.h"
#include "CRTColor.h"
#include "Light.h"
#include "CRTMatrix.h"
#include "Material.h"

// Small bias used to nudge secondary ray origins off the surface they were
// spawned from, avoiding self-intersection ("shadow acne" / reflection acne)
// due to floating point error.
static const float RAY_BIAS = 1e-3f;
static const int MAX_REFLECTION_DEPTH = 5;

struct SceneHit {
    const CRTTriangle* triangle = nullptr;
    float t = std::numeric_limits<float>::max();
    float u = 0.0f, v = 0.0f, w = 0.0f;
};

// Finds the closest triangle hit by the ray, if any.
inline SceneHit intersectScene(const Ray& ray, const std::vector<CRTTriangle>& triangles) {
    SceneHit closest;
    for (const CRTTriangle& tri : triangles) {
        HitInfo hit;
        if (intersectTriangle(ray, tri, hit)) {
            if (hit.t < closest.t) {
                closest.t = hit.t;
                closest.triangle = &tri;
                closest.u = hit.u;
                closest.v = hit.v;
                closest.w = hit.w;
            }
        }
    }
    return closest;
}

// Checks if `point` is occluded from `light` along the given (already normalized)
// direction and distance. Caller computes lightDir/dist once and shares them with
// the shading pass, avoiding a redundant length()/normalize() per light per hit.
inline bool isInShadow(const CRTVector& point, const CRTVector& normal,
                        const CRTVector& lightDir, float distToLight,
                        const std::vector<CRTTriangle>& triangles) {
    CRTVector shadowOrigin = point + normal * RAY_BIAS;
    Ray shadowRay(shadowOrigin, lightDir);

    for (const CRTTriangle& tri : triangles) {
        HitInfo hit;
        if (intersectTriangle(shadowRay, tri, hit)) {
            if (hit.t < distToLight) {
                return true;
            }
        }
    }
    return false;
}

// Lambertian (diffuse) direct lighting contribution from all lights, in linear [0,1]-ish space.
inline CRTVector shadeDiffuse(const CRTVector& point, const CRTVector& normal, const CRTVector& albedo,
                               const std::vector<Light>& lights, const std::vector<CRTTriangle>& triangles) {
    CRTVector result(0.0f, 0.0f, 0.0f);

    for (const Light& light : lights) {
        CRTVector toLight = light.position - point;
        float dist = toLight.length();
        if (dist < 1e-8f) {
            continue; // Degenerate: light sits on the shading point, no well-defined direction.
        }
        CRTVector lightDir = toLight * (1.0f / dist);

        if (isInShadow(point, normal, lightDir, dist, triangles)) {
            continue;
        }

        float cosLaw = std::max(0.0f, dot(normal, lightDir));
        float attenuation = light.intensity / (4.0f * PI * dist * dist);

        result += albedo * (attenuation * cosLaw);
    }

    return result;
}

// Forward declaration: traceRay and shadeHit are mutually recursive because
// reflective materials need to trace a new ray and shade whatever it hits.
inline CRTVector traceRay(const Ray& ray, const std::vector<CRTTriangle>& triangles,
                           const std::vector<Light>& lights, const std::vector<Material>& materials,
                           const CRTVector& backgroundColor, int depth);

inline CRTVector shadeHit(const Ray& ray, const SceneHit& hitResult, const std::vector<CRTTriangle>& triangles,
                           const std::vector<Light>& lights, const std::vector<Material>& materials,
                           const CRTVector& backgroundColor, int depth) {
    const CRTTriangle& tri = *hitResult.triangle;
    const Material& material =
        (tri.materialIndex >= 0 && static_cast<size_t>(tri.materialIndex) < materials.size())
            ? materials[tri.materialIndex]
            : Material{};

    CRTVector hitPoint = ray.origin + ray.direction * hitResult.t;
    CRTVector normal = material.smoothShading
        ? tri.smoothNormalAt(hitResult.u, hitResult.v, hitResult.w)
        : tri.normal;

    // Keep the shading normal on the same side as the incoming ray so both
    // diffuse lighting and reflections behave correctly if we ever hit a
    // back face (e.g. rays bouncing inside a closed mesh).
    if (dot(normal, ray.direction) > 0.0f) {
        normal = -normal;
    }

    if (material.type == MaterialType::Reflective) {
        if (depth >= MAX_REFLECTION_DEPTH) {
            return CRTVector(0.0f, 0.0f, 0.0f);
        }
        CRTVector reflectedDir = reflect(ray.direction, normal).normalize();
        CRTVector reflectedOrigin = hitPoint + normal * RAY_BIAS;
        Ray reflectedRay(reflectedOrigin, reflectedDir);
        CRTVector reflectedColor = traceRay(reflectedRay, triangles, lights, materials, backgroundColor, depth + 1);
        // Tint the reflection by the material's albedo (acts as the mirror's reflectivity/tint).
        return reflectedColor * material.albedo;
    }

    // Diffuse (default) material.
    return shadeDiffuse(hitPoint, normal, material.albedo, lights, triangles);
}

inline CRTVector traceRay(const Ray& ray, const std::vector<CRTTriangle>& triangles,
                           const std::vector<Light>& lights, const std::vector<Material>& materials,
                           const CRTVector& backgroundColor, int depth) {
    SceneHit hitResult = intersectScene(ray, triangles);
    if (!hitResult.triangle) {
        return backgroundColor;
    }
    return shadeHit(ray, hitResult, triangles, lights, materials, backgroundColor, depth);
}

enum class RenderMode {
    // Full shading: diffuse lighting + shadows + reflections, driven by materials.
    Shaded,
    // Debug mode for HW09 Task 1: color = barycentric weights (u,v,w) as RGB, ignoring materials/lights.
    Barycentric
};

inline void writePpmHeader(std::ofstream& out, int width, int height) {
    out << "P3\n" << width << " " << height << "\n255\n";
}

inline void renderScene(const Camera& camera, const std::vector<CRTTriangle>& triangles,
                         int width, int height, const CRTColor& backgroundColor,
                         const std::string& outputPath, const std::vector<Light>& lights = {},
                         const std::vector<Material>& materials = {},
                         RenderMode mode = RenderMode::Shaded) {
    std::ofstream ppmFileStream(outputPath, std::ios::out | std::ios::binary);
    writePpmHeader(ppmFileStream, width, height);

    CRTVector bgLinear(backgroundColor.r / 255.0f, backgroundColor.g / 255.0f, backgroundColor.b / 255.0f);

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            Ray ray = camera.generateRay(colIdx, rowIdx, width, height);

            CRTColor pixelColor;

            if (mode == RenderMode::Barycentric) {
                SceneHit hitResult = intersectScene(ray, triangles);
                if (!hitResult.triangle) {
                    pixelColor = backgroundColor;
                } else {
                    pixelColor = CRTColor::fromLinear(CRTVector(hitResult.u, hitResult.v, hitResult.w));
                }
            } else {
                CRTVector linearColor = traceRay(ray, triangles, lights, materials, bgLinear, 0);
                pixelColor = CRTColor::fromLinear(linearColor);
            }

            ppmFileStream << pixelColor.r << " " << pixelColor.g << " " << pixelColor.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}
