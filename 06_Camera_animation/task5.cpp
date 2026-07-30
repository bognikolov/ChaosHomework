#include "hw06_core.h"

int main() {
    std::vector<CRTTriangle> scene;
    scene.push_back(CRTTriangle(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f),
        CRTColor(220, 60, 60)
    ));
    scene.push_back(CRTTriangle(
        CRTVector(-4.5f, -1.0f, -6.0f),
        CRTVector(-2.5f, -1.0f, -6.0f),
        CRTVector(-3.5f, 1.0f, -6.0f),
        CRTColor(60, 220, 60)
    ));
    scene.push_back(CRTTriangle(
        CRTVector(2.5f, -1.0f, -6.0f),
        CRTVector(4.5f, -1.0f, -6.0f),
        CRTVector(3.5f, 1.0f, -6.0f),
        CRTColor(60, 60, 220)
    ));

    const int frameCount = 72;
    const float panPerFrame = 5.0f;
    const int width = 320;
    const int height = 240;

    Camera camera;

    for (int frame = 0; frame < frameCount; ++frame) {
        char filename[64];
        std::snprintf(filename, sizeof(filename), "frame_%03d.ppm", frame);
        renderScene(camera, scene, width, height, filename);
        camera.pan(panPerFrame);
    }

    std::cout << "Generated " << frameCount << " frames.\n";
    return 0;
}
