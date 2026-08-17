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
#include "Texture.h"

// Bias so a bounced ray doesn't self-intersect the surface it just left.
// Reflection/refraction need more headroom than shadow rays since they
// often travel near thin geometry.
static const float SHADOW_BIAS = 0.01f;
static const float REFLECTION_BIAS = 0.1f;
static const float REFRACTION_BIAS = 0.1f;
static const int MAX_TRACE_DEPTH = 5;
static const float AIR_IOR = 1.0f;

struct SceneHit {
    const CRTTriangle* triangle = nullptr;
    float t = std::numeric_limits<float>::max();
    float u = 0.0f, v = 0.0f, w = 0.0f;
};

// Shadow rays skip refractive triangles - we don't bend shadow rays through
// glass, so treating it as a solid occluder would just cast a wrong black
// shadow instead of the dimmed/bent light that should get through.
inline SceneHit intersectScene(const Ray& ray, const std::vector<CRTTriangle>& triangles,
                                const std::vector<Material>& materials) {
    SceneHit closest;
    for (const CRTTriangle& tri : triangles) {
        if (ray.type == RayType::Shadow &&
            tri.materialIndex >= 0 && static_cast<size_t>(tri.materialIndex) < materials.size() &&
            materials[tri.materialIndex].type == MaterialType::Refractive) {
            continue;
        }
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

inline bool isInShadow(const CRTVector& point, const CRTVector& normal,
                        const CRTVector& lightDir, float distToLight,
                        const std::vector<CRTTriangle>& triangles,
                        const std::vector<Material>& materials) {
    CRTVector shadowOrigin = point + normal * SHADOW_BIAS;
    Ray shadowRay(shadowOrigin, lightDir, AIR_IOR, RayType::Shadow);

    for (const CRTTriangle& tri : triangles) {
        if (tri.materialIndex >= 0 && static_cast<size_t>(tri.materialIndex) < materials.size() &&
            materials[tri.materialIndex].type == MaterialType::Refractive) {
            continue;
        }
        HitInfo hit;
        if (intersectTriangle(shadowRay, tri, hit)) {
            if (hit.t < distToLight) {
                return true;
            }
        }
    }
    return false;
}

inline CRTVector resolveAlbedo(const Material& material, const std::vector<Texture>& textures,
                                const CRTTriangle& tri, float u, float v, float w) {
    if (material.textureIndex < 0 || static_cast<size_t>(material.textureIndex) >= textures.size()) {
        return material.albedo;
    }
    float texU, texV;
    tri.uvAt(u, v, w, texU, texV);
    return textures[material.textureIndex].sample(u, v, w, texU, texV);
}

inline CRTVector shadeDiffuse(const CRTVector& point, const CRTVector& normal, const CRTVector& albedo,
                               const std::vector<Light>& lights, const std::vector<CRTTriangle>& triangles,
                               const std::vector<Material>& materials) {
    CRTVector result(0.0f, 0.0f, 0.0f);

    for (const Light& light : lights) {
        CRTVector toLight = light.position - point;
        float dist = toLight.length();
        if (dist < 1e-8f) {
            continue;
        }
        CRTVector lightDir = toLight * (1.0f / dist);

        if (isInShadow(point, normal, lightDir, dist, triangles, materials)) {
            continue;
        }

        float cosLaw = std::max(0.0f, dot(normal, lightDir));
        float attenuation = light.intensity / (4.0f * PI * dist * dist);

        result += albedo * (attenuation * cosLaw);
    }

    // clamp so a very bright/close light doesn't blow past 1.0 and then
    // get amplified further by reflection/refraction bounces above this
    result.x = std::min(result.x, 1.0f);
    result.y = std::min(result.y, 1.0f);
    result.z = std::min(result.z, 1.0f);

    return result;
}

inline CRTVector traceRay(const Ray& ray, const std::vector<CRTTriangle>& triangles,
                           const std::vector<Light>& lights, const std::vector<Material>& materials,
                           const std::vector<Texture>& textures,
                           const CRTVector& backgroundColor, int depth);

inline CRTVector shadeHit(const Ray& ray, const SceneHit& hitResult, const std::vector<CRTTriangle>& triangles,
                           const std::vector<Light>& lights, const std::vector<Material>& materials,
                           const std::vector<Texture>& textures,
                           const CRTVector& backgroundColor, int depth) {
    const CRTTriangle& tri = *hitResult.triangle;
    const Material& material =
        (tri.materialIndex >= 0 && static_cast<size_t>(tri.materialIndex) < materials.size())
            ? materials[tri.materialIndex]
            : Material{};

    CRTVector hitPoint = ray.origin + ray.direction * hitResult.t;
    CRTVector geometricNormal = material.smoothShading
        ? tri.smoothNormalAt(hitResult.u, hitResult.v, hitResult.w)
        : tri.normal;

    bool entering = dot(ray.direction, geometricNormal) < 0.0f;
    CRTVector normal = entering ? geometricNormal : -geometricNormal;

    if (material.type == MaterialType::Constant) {
        return resolveAlbedo(material, textures, tri, hitResult.u, hitResult.v, hitResult.w);
    }

    if (material.type == MaterialType::Reflective) {
        CRTVector reflectedDir = reflect(ray.direction, normal).normalize();
        CRTVector reflectedOrigin = hitPoint + normal * REFLECTION_BIAS;
        Ray reflectedRay(reflectedOrigin, reflectedDir, ray.ior, RayType::Reflection);
        CRTVector reflectedColor = traceRay(reflectedRay, triangles, lights, materials, textures, backgroundColor, depth + 1);
        return reflectedColor * resolveAlbedo(material, textures, tri, hitResult.u, hitResult.v, hitResult.w);
    }

    if (material.type == MaterialType::Refractive) {
        float iorFrom = ray.ior;
        float iorTo = entering ? material.ior : AIR_IOR;
        float eta = iorFrom / iorTo;

        CRTVector incomingDir = ray.direction.normalize();
        float cosThetaI = std::max(0.0f, -dot(incomingDir, normal));

        CRTVector refractedDir;
        bool canRefract = refract(incomingDir, normal, eta, refractedDir);

        float reflectance = canRefract ? fresnelSchlick(cosThetaI, iorFrom, iorTo) : 1.0f;

        CRTVector reflectedDir = reflect(incomingDir, normal).normalize();
        CRTVector reflectedOrigin = hitPoint + normal * REFLECTION_BIAS;
        Ray reflectedRay(reflectedOrigin, reflectedDir, iorFrom, RayType::Reflection);
        CRTVector reflectedColor = traceRay(reflectedRay, triangles, lights, materials, textures, backgroundColor, depth + 1);

        if (!canRefract) {
            return reflectedColor;
        }

        CRTVector refractedOrigin = hitPoint - normal * REFRACTION_BIAS;
        Ray refractedRay(refractedOrigin, refractedDir.normalize(), iorTo, RayType::Refraction);
        CRTVector refractedColor = traceRay(refractedRay, triangles, lights, materials, textures, backgroundColor, depth + 1);

        return reflectedColor * reflectance + refractedColor * (1.0f - reflectance);
    }

    CRTVector albedo = resolveAlbedo(material, textures, tri, hitResult.u, hitResult.v, hitResult.w);
    return shadeDiffuse(hitPoint, normal, albedo, lights, triangles, materials);
}

inline CRTVector traceRay(const Ray& ray, const std::vector<CRTTriangle>& triangles,
                           const std::vector<Light>& lights, const std::vector<Material>& materials,
                           const std::vector<Texture>& textures,
                           const CRTVector& backgroundColor, int depth) {
    if (depth >= MAX_TRACE_DEPTH) {
        return backgroundColor;
    }
    SceneHit hitResult = intersectScene(ray, triangles, materials);
    if (!hitResult.triangle) {
        return backgroundColor;
    }
    return shadeHit(ray, hitResult, triangles, lights, materials, textures, backgroundColor, depth);
}

enum class RenderMode {
    Shaded,
    Barycentric
};

inline void writePpmHeader(std::ofstream& out, int width, int height) {
    out << "P3\n" << width << " " << height << "\n255\n";
}

inline void renderScene(const Camera& camera, const std::vector<CRTTriangle>& triangles,
                         int width, int height, const CRTColor& backgroundColor,
                         const std::string& outputPath, const std::vector<Light>& lights = {},
                         const std::vector<Material>& materials = {},
                         const std::vector<Texture>& textures = {},
                         RenderMode mode = RenderMode::Shaded) {
    std::ofstream ppmFileStream(outputPath, std::ios::out | std::ios::binary);
    writePpmHeader(ppmFileStream, width, height);

    CRTVector bgLinear(backgroundColor.r / 255.0f, backgroundColor.g / 255.0f, backgroundColor.b / 255.0f);

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            Ray ray = camera.generateRay(colIdx, rowIdx, width, height);

            CRTColor pixelColor;

            if (mode == RenderMode::Barycentric) {
                SceneHit hitResult = intersectScene(ray, triangles, materials);
                if (!hitResult.triangle) {
                    pixelColor = backgroundColor;
                } else {
                    pixelColor = CRTColor::fromLinear(CRTVector(hitResult.u, hitResult.v, hitResult.w));
                }
            } else {
                CRTVector linearColor = traceRay(ray, triangles, lights, materials, textures, bgLinear, 0);
                pixelColor = CRTColor::fromLinear(linearColor);
            }

            ppmFileStream << pixelColor.r << " " << pixelColor.g << " " << pixelColor.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}
