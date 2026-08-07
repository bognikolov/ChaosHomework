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

inline bool isInShadow(const CRTVector& point, const CRTVector& normal, const Light& light,
                        const std::vector<CRTTriangle>& triangles) {
    CRTVector toLight = light.position - point;
    float distToLight = toLight.length();
    CRTVector shadowDir = toLight.normalize();

    CRTVector shadowOrigin = point + normal * 1e-3f;
    Ray shadowRay(shadowOrigin, shadowDir);

    for (const CRTTriangle& tri : triangles) {
        float t;
        if (intersectTriangle(shadowRay, tri, t)) {
            if (t < distToLight) {
                return true;
            }
        }
    }
    return false;
}

inline CRTColor shadePoint(const CRTVector& point, const CRTVector& normal, const CRTVector& albedo,
                            const std::vector<Light>& lights, const std::vector<CRTTriangle>& triangles) {
    float r = 0.0f, g = 0.0f, b = 0.0f;

    for (const Light& light : lights) {
        if (isInShadow(point, normal, light, triangles)) {
            continue;
        }

        CRTVector toLight = light.position - point;
        float dist = toLight.length();
        CRTVector lightDir = toLight.normalize();

        float cosLaw = std::max(0.0f, dot(normal, lightDir));
        float attenuation = light.intensity / (4.0f * PI * dist * dist);

        r += albedo.x * attenuation * cosLaw;
        g += albedo.y * attenuation * cosLaw;
        b += albedo.z * attenuation * cosLaw;
    }

    return CRTColor(
        std::clamp(static_cast<int>(r * 255.0f), 0, 255),
        std::clamp(static_cast<int>(g * 255.0f), 0, 255),
        std::clamp(static_cast<int>(b * 255.0f), 0, 255)
    );
}

inline void renderScene(const Camera& camera, const std::vector<CRTTriangle>& triangles,
                         int width, int height, const CRTColor& backgroundColor,
                         const std::string& outputPath, const std::vector<Light>& lights = {}) {
    std::ofstream ppmFileStream(outputPath, std::ios::out | std::ios::binary);
    ppmFileStream << "P3\n" << width << " " << height << "\n255\n";

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            Ray ray = camera.generateRay(colIdx, rowIdx, width, height);

            float closestT = std::numeric_limits<float>::max();
            const CRTTriangle* closestTri = nullptr;

            for (const CRTTriangle& tri : triangles) {
                float t;
                if (intersectTriangle(ray, tri, t)) {
                    if (t < closestT) {
                        closestT = t;
                        closestTri = &tri;
                    }
                }
            }

            CRTColor pixelColor;
            if (!closestTri) {
                pixelColor = backgroundColor;
            } else if (lights.empty()) {
                pixelColor = closestTri->color;
            } else {
                CRTVector hitPoint = ray.origin + ray.direction * closestT;
                pixelColor = shadePoint(hitPoint, closestTri->normal, closestTri->albedo, lights, triangles);
            }

            ppmFileStream << pixelColor.r << " " << pixelColor.g << " " << pixelColor.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}
