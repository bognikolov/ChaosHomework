#pragma once
#include <fstream>
#include <vector>
#include <limits>
#include <string>
#include "Camera.h"
#include "Triangle.h"
#include "CRTColor.h"

inline void renderScene(const Camera& camera, const std::vector<CRTTriangle>& triangles,
                         int width, int height, const CRTColor& backgroundColor,
                         const std::string& outputPath) {
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

            CRTColor pixelColor = closestTri ? closestTri->color : backgroundColor;
            ppmFileStream << pixelColor.r << " " << pixelColor.g << " " << pixelColor.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}
