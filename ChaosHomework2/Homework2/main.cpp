#include <fstream>
#include <cmath>
#include <algorithm>

struct CRTVector {
    float x, y, z;

    CRTVector() : x(0), y(0), z(0) {}
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
    }
};

struct CRTColor {
    int r, g, b;

    CRTColor() : r(0), g(0), b(0) {}
    CRTColor(int r, int g, int b) : r(r), g(g), b(b) {}
};

static const int imageWidth = 1920;
static const int imageHeight = 1080;
static const int maxColorComponent = 255;

CRTVector generateRayDirection(int x, int y, int width, int height) {
    float fx = static_cast<float>(x) + 0.5f;
    float fy = static_cast<float>(y) + 0.5f;

    fx /= static_cast<float>(width);
    fy /= static_cast<float>(height);

    fx = (2.0f * fx) - 1.0f;
    fy = 1.0f - (2.0f * fy);

    fx *= static_cast<float>(width) / static_cast<float>(height);

    CRTVector dir(fx, fy, -1.0f);
    return dir.normalize();
}

int main() {
    std::ofstream ppmFileStream("crt_output_image.ppm", std::ios::out | std::ios::binary);
    ppmFileStream << "P3\n";
    ppmFileStream << imageWidth << " " << imageHeight << "\n";
    ppmFileStream << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            CRTVector rayDir = generateRayDirection(colIdx, rowIdx, imageWidth, imageHeight);

            CRTColor color(
                std::clamp(static_cast<int>((rayDir.x * 0.5f + 0.5f) * maxColorComponent), 0, maxColorComponent),
                std::clamp(static_cast<int>((rayDir.y * 0.5f + 0.5f) * maxColorComponent), 0, maxColorComponent),
                std::clamp(static_cast<int>((rayDir.z * 0.5f + 0.5f) * maxColorComponent), 0, maxColorComponent)
            );

            ppmFileStream << color.r << " " << color.g << " " << color.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
    return 0;
}