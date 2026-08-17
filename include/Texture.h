#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>
#include "CRTVector.h"

enum class TextureType {
    Albedo,
    Edges,
    Checker,
    Bitmap
};

inline TextureType textureTypeFromString(const std::string& s) {
    if (s == "edges") return TextureType::Edges;
    if (s == "checker") return TextureType::Checker;
    if (s == "bitmap") return TextureType::Bitmap;
    return TextureType::Albedo;
}

struct Texture {
    std::string name;
    TextureType type = TextureType::Albedo;

    // albedo
    CRTVector albedo = CRTVector(0.7f, 0.7f, 0.7f);

    // edges
    CRTVector edgeColor = CRTVector(0.0f, 0.0f, 0.0f);
    CRTVector innerColor = CRTVector(1.0f, 1.0f, 1.0f);
    float edgeWidth = 0.05f;

    // checker
    CRTVector colorA = CRTVector(0.0f, 0.0f, 0.0f);
    CRTVector colorB = CRTVector(1.0f, 1.0f, 1.0f);
    float squareSize = 0.25f;

    // bitmap
    std::vector<unsigned char> pixels;
    int width = 0;
    int height = 0;
    int channels = 0;

    CRTVector bitmapPixel(int x, int y) const {
        x = std::clamp(x, 0, width - 1);
        y = std::clamp(y, 0, height - 1);
        int idx = (y * width + x) * channels;
        return CRTVector(pixels[idx] / 255.0f, pixels[idx + 1] / 255.0f, pixels[idx + 2] / 255.0f);
    }

    // u,v,w: barycentric weights at the hit point (for Edges).
    // texU,texV: UV texture coordinates at the hit point (for Checker/Bitmap).
    CRTVector sample(float u, float v, float w, float texU, float texV) const {
        switch (type) {
            case TextureType::Albedo:
                return albedo;

            case TextureType::Edges: {
                float minBary = std::min(u, std::min(v, w));
                return minBary < edgeWidth ? edgeColor : innerColor;
            }

            case TextureType::Checker: {
                int cx = static_cast<int>(std::floor(texU / squareSize));
                int cy = static_cast<int>(std::floor(texV / squareSize));
                bool isA = ((cx + cy) % 2 + 2) % 2 == 0;
                return isA ? colorA : colorB;
            }

            case TextureType::Bitmap: {
                if (pixels.empty()) {
                    return CRTVector(1.0f, 0.0f, 1.0f); // missing texture -> magenta
                }
                float px = texU * static_cast<float>(width);
                float py = (1.0f - texV) * static_cast<float>(height);
                int x = static_cast<int>(px);
                int y = static_cast<int>(py);
                return bitmapPixel(x, y);
            }
        }
        return CRTVector(0.0f, 0.0f, 0.0f);
    }
};
