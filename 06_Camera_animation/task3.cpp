#include "hw06_core.h"

std::vector<CRTTriangle> buildScene() {
    std::vector<CRTTriangle> scene;
    scene.push_back(CRTTriangle(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f),
        CRTColor(220, 60, 60)
    ));
    scene.push_back(CRTTriangle(
        CRTVector(-3.0f, -1.0f, -6.0f),
        CRTVector(-1.0f, -1.0f, -6.0f),
        CRTVector(-2.0f, 1.0f, -6.0f),
        CRTColor(60, 220, 60)
    ));
    return scene;
}

void demoMovement(const std::string& name, void (Camera::*movement)(float), float amount) {
    std::vector<CRTTriangle> scene = buildScene();
    Camera camera;

    renderScene(camera, scene, 640, 480, name + "_before.ppm");

    (camera.*movement)(amount);

    renderScene(camera, scene, 640, 480, name + "_after.ppm");

    std::cout << name << " done, camera position: " << camera.position << "\n";
}

int main() {
    demoMovement("dolly", &Camera::dolly, -1.5f);
    demoMovement("truck", &Camera::truck, 1.0f);
    demoMovement("pedestal", &Camera::pedestal, 0.8f);
    demoMovement("pan", &Camera::pan, 20.0f);
    demoMovement("tilt", &Camera::tilt, 15.0f);
    demoMovement("roll", &Camera::roll, 25.0f);

    return 0;
}
