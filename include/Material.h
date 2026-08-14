#pragma once
#include <string>
#include "CRTVector.h"

enum class MaterialType {
    Diffuse,
    Reflective,
    Refractive,
    Constant
};

inline MaterialType materialTypeFromString(const std::string& s) {
    if (s == "reflective") {
        return MaterialType::Reflective;
    }
    if (s == "refractive") {
        return MaterialType::Refractive;
    }
    if (s == "constant") {
        return MaterialType::Constant;
    }
    return MaterialType::Diffuse;
}

struct Material {
    MaterialType type = MaterialType::Diffuse;
    CRTVector albedo = CRTVector(0.7f, 0.7f, 0.7f);
    bool smoothShading = false;
    // Index of refraction, only meaningful for Refractive materials. Defaults
    // to a common glass-like value when a scene omits it.
    float ior = 1.5f;
};
