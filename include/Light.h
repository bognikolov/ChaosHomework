#pragma once
#include "CRTVector.h"

struct Light {
    CRTVector position;
    float intensity;

    Light() : intensity(0.0f) {}
    Light(const CRTVector& position, float intensity)
        : position(position), intensity(intensity) {}
};
