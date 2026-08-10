#pragma once
#include <string>
#include "CRTVector.h"

enum class MaterialType {
    Diffuse,
    Reflective
};

inline MaterialType materialTypeFromString(const std::string& s) {
    if (s == "reflective") {
        return MaterialType::Reflective;
    }
    return MaterialType::Diffuse;
}

struct Material {
    MaterialType type = MaterialType::Diffuse;
    CRTVector albedo = CRTVector(0.7f, 0.7f, 0.7f);
    bool smoothShading = false;
};
