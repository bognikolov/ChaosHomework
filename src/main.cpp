#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "CRTVector.h"
#include "CRTMatrix.h"
#include "Triangle.h"
#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"

void runRays(int width, int height) {
    Camera camera;
    std::vector<CRTTriangle> noTriangles;

    std::ofstream ppmFileStream("output_rays.ppm", std::ios::out | std::ios::binary);
    ppmFileStream << "P3\n" << width << " " << height << "\n255\n";

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            Ray ray = camera.generateRay(colIdx, rowIdx, width, height);
            CRTVector dir = ray.direction;

            int r = std::clamp(static_cast<int>((dir.x * 0.5f + 0.5f) * 255), 0, 255);
            int g = std::clamp(static_cast<int>((dir.y * 0.5f + 0.5f) * 255), 0, 255);
            int b = std::clamp(static_cast<int>((dir.z * 0.5f + 0.5f) * 255), 0, 255);

            ppmFileStream << r << " " << g << " " << b << "\t";
        }
        ppmFileStream << "\n";
    }
    ppmFileStream.close();
    std::cout << "HW03 - Rays: rendered output_rays.ppm\n";
}

void runTriangleMath() {
    CRTVector a1(3.5f, 0.0f, 0.0f), b1(1.75f, 3.5f, 0.0f);
    CRTVector c1 = cross(a1, b1);
    assert(almostEqual(c1.x, 0.0f) && almostEqual(c1.y, 0.0f) && almostEqual(c1.z, 12.25f));
    std::cout << "Cross 1: " << c1 << "\n";

    CRTVector a2(3.0f, -3.0f, 1.0f), b2(4.0f, 9.0f, 3.0f);
    CRTVector c2 = cross(a2, b2);
    assert(almostEqual(c2.length(), 43.2435f, 1e-3f));
    std::cout << "Cross 2: " << c2 << " Area: " << c2.length() << "\n";

    CRTTriangle t1(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f),
        CRTColor(220, 60, 60)
    );
    assert(almostEqual(t1.area(), 6.125f, 1e-3f));
    std::cout << "Triangle normal: " << t1.normal << " Area: " << t1.area() << "\n";

    std::cout << "HW04 - Triangle math: all assertions passed\n";
}

void runClosestHit(int width, int height) {
    std::vector<CRTTriangle> scene;
    CRTVector apex(0.0f, 1.2f, -4.0f);
    CRTVector base1(-1.3f, -0.8f, -3.0f);
    CRTVector base2(1.3f, -0.8f, -3.0f);
    CRTVector base3(0.0f, -0.8f, -5.0f);

    scene.push_back(CRTTriangle(apex, base1, base2, CRTColor(220, 60, 60)));
    scene.push_back(CRTTriangle(apex, base2, base3, CRTColor(60, 220, 60)));
    scene.push_back(CRTTriangle(apex, base3, base1, CRTColor(60, 60, 220)));
    scene.push_back(CRTTriangle(base1, base3, base2, CRTColor(220, 220, 60)));

    Camera camera;
    renderScene(camera, scene, width, height, CRTColor(30, 30, 30), "output_intersect.ppm");
    std::cout << "HW05 - Closest hit: rendered output_intersect.ppm\n";
}

void runCameraDemo(int width, int height) {
    std::vector<CRTTriangle> scene;
    scene.push_back(CRTTriangle(
        CRTVector(-1.75f, -1.75f, -3.0f), CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f), CRTColor(220, 60, 60)
    ));
    scene.push_back(CRTTriangle(
        CRTVector(-3.5f, -1.0f, -6.0f), CRTVector(-1.5f, -1.0f, -6.0f),
        CRTVector(-2.5f, 1.0f, -6.0f), CRTColor(60, 220, 60)
    ));

    Camera camera;
    CRTColor bg(30, 30, 30);
    renderScene(camera, scene, width, height, bg, "output_camera_before.ppm");

    camera.pan(25.0f);
    camera.tilt(-10.0f);
    camera.truck(0.8f);
    camera.dolly(-0.5f);

    renderScene(camera, scene, width, height, bg, "output_camera_after.ppm");
    std::cout << "HW06 - Camera movement: rendered before/after images\n";
}

void runSceneFile(const std::string& inputPath, const std::string& outputPath) {
    SceneData scene = loadScene(inputPath);
    std::cout << "HW07 - Scene: loaded " << scene.triangles.size() << " triangles, "
              << scene.imageWidth << "x" << scene.imageHeight << "\n";

    renderScene(scene.camera, scene.triangles, scene.imageWidth, scene.imageHeight,
                scene.backgroundColor, outputPath);
    std::cout << "Rendered to " << outputPath << "\n";
}

void printUsage() {
    std::cout << "Usage:\n"
              << "  ChaosRayTracer rays\n"
              << "  ChaosRayTracer triangle\n"
              << "  ChaosRayTracer intersect\n"
              << "  ChaosRayTracer camera-demo\n"
              << "  ChaosRayTracer scene <input.crtscene> <output.ppm>\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string command = argv[1];

    if (command == "rays") {
        runRays(1280, 720);
    } else if (command == "triangle") {
        runTriangleMath();
    } else if (command == "intersect") {
        runClosestHit(1280, 720);
    } else if (command == "camera-demo") {
        runCameraDemo(640, 480);
    } else if (command == "scene") {
        if (argc < 4) {
            std::cout << "scene command requires <input.crtscene> <output.ppm>\n";
            return 1;
        }
        runSceneFile(argv[2], argv[3]);
    } else {
        printUsage();
        return 1;
    }

    return 0;
}
