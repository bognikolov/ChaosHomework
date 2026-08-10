#pragma once
#include <algorithm>
#include "CRTVector.h"

struct CRTColor {
    int r, g, b;
    CRTColor() : r(0), g(0), b(0) {}
    CRTColor(int r, int g, int b) : r(r), g(g), b(b) {}

    // Builds a CRTColor from a linear [0,1] color vector, clamping to [0,255].
    static CRTColor fromLinear(const CRTVector& c) {
        return CRTColor(
            std::clamp(static_cast<int>(c.x * 255.0f), 0, 255),
            std::clamp(static_cast<int>(c.y * 255.0f), 0, 255),
            std::clamp(static_cast<int>(c.z * 255.0f), 0, 255)
        );
    }
};
