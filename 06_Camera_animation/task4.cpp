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
        CRTVector(-3.5f, -1.0f, -6.0f),
        CRTVector(-1.5f, -1.0f, -6.0f),
        CRTVector(-2.5f, 1.0f, -6.0f),
        CRTColor(60, 220, 60)
    ));
    scene.push_back(CRTTriangle(
        CRTVector(1.5f, -1.0f, -7.0f),
        CRTVector(3.5f, -1.0f, -7.0f),
        CRTVector(2.5f, 1.0f, -7.0f),
        CRTColor(60, 60, 220)
    ));

    Camera camera;
    renderScene(camera, scene, 640, 480, "combo_before.ppm");

    camera.pan(25.0f);
    camera.tilt(-10.0f);
    camera.truck(0.8f);
    camera.dolly(-0.5f);

    renderScene(camera, scene, 640, 480, "combo_after.ppm");

    std::cout << "Final camera position: " << camera.position << "\n";
    std::cout << "Final forward direction: " << camera.forward() << "\n";

    return 0;
}
