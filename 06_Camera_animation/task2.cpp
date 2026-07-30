#include "hw06_core.h"

int main() {
    std::vector<CRTTriangle> scene;
    scene.push_back(CRTTriangle(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f),
        CRTColor(220, 60, 60)
    ));

    Camera camera(CRTVector(0.5f, 0.3f, 1.5f));

    renderScene(camera, scene, 800, 600, "task2_output.ppm");
    std::cout << "Rendered task2_output.ppm from camera at " << camera.position << "\n";

    return 0;
}
