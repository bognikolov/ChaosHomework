#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <chrono>
#include "CRTVector.h"
#include "CRTMatrix.h"
#include "Triangle.h"
#include "Material.h"
#include "Camera.h"
#include "Renderer.h"
#include "Scene.h"

void runRays(int width, int height) {
    Camera camera;

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
        CRTVector(0.0f, 1.75f, -3.0f)
    );
    assert(almostEqual(t1.area(), 6.125f, 1e-3f));
    std::cout << "Triangle normal: " << t1.normal << " Area: " << t1.area() << "\n";

    std::cout << "HW04 - Triangle math: all assertions passed\n";
}

void runClosestHit(int width, int height) {
    std::vector<CRTTriangle> scene;
    std::vector<Material> materials = {
        Material{MaterialType::Diffuse, CRTVector(220 / 255.0f, 60 / 255.0f, 60 / 255.0f), false},
        Material{MaterialType::Diffuse, CRTVector(60 / 255.0f, 220 / 255.0f, 60 / 255.0f), false},
        Material{MaterialType::Diffuse, CRTVector(60 / 255.0f, 60 / 255.0f, 220 / 255.0f), false},
        Material{MaterialType::Diffuse, CRTVector(220 / 255.0f, 220 / 255.0f, 60 / 255.0f), false},
    };

    CRTVector apex(0.0f, 1.2f, -4.0f);
    CRTVector base1(-1.3f, -0.8f, -3.0f);
    CRTVector base2(1.3f, -0.8f, -3.0f);
    CRTVector base3(0.0f, -0.8f, -5.0f);

    scene.push_back(CRTTriangle(apex, base1, base2, 0));
    scene.push_back(CRTTriangle(apex, base2, base3, 1));
    scene.push_back(CRTTriangle(apex, base3, base1, 2));
    scene.push_back(CRTTriangle(base1, base3, base2, 3));

    std::vector<Light> lights = { Light(CRTVector(0.0f, 2.0f, 0.0f), 50.0f) };

    Camera camera;
    renderScene(camera, scene, width, height, CRTColor(30, 30, 30), "output_intersect.ppm", lights, materials);
    std::cout << "HW05 - Closest hit: rendered output_intersect.ppm\n";
}

void runCameraDemo(int width, int height) {
    std::vector<CRTTriangle> scene;
    std::vector<Material> materials = {
        Material{MaterialType::Diffuse, CRTVector(220 / 255.0f, 60 / 255.0f, 60 / 255.0f), false},
        Material{MaterialType::Diffuse, CRTVector(60 / 255.0f, 220 / 255.0f, 60 / 255.0f), false},
    };

    scene.push_back(CRTTriangle(
        CRTVector(-1.75f, -1.75f, -3.0f), CRTVector(1.75f, -1.75f, -3.0f),
        CRTVector(0.0f, 1.75f, -3.0f), 0
    ));
    scene.push_back(CRTTriangle(
        CRTVector(-3.5f, -1.0f, -6.0f), CRTVector(-1.5f, -1.0f, -6.0f),
        CRTVector(-2.5f, 1.0f, -6.0f), 1
    ));

    std::vector<Light> lights = { Light(CRTVector(0.0f, 2.0f, 2.0f), 50.0f) };

    Camera camera;
    CRTColor bg(30, 30, 30);
    renderScene(camera, scene, width, height, bg, "output_camera_before.ppm", lights, materials);

    camera.pan(25.0f);
    camera.tilt(-10.0f);
    camera.truck(0.8f);
    camera.dolly(-0.5f);

    renderScene(camera, scene, width, height, bg, "output_camera_after.ppm", lights, materials);
    std::cout << "HW06 - Camera movement: rendered before/after images\n";
}

void runSceneFile(const std::string& inputPath, const std::string& outputPath) {
    SceneData scene = loadScene(inputPath);
    std::cout << "Scene: loaded " << scene.triangles.size() << " triangles, "
              << scene.lights.size() << " lights, " << scene.materials.size() << " materials, "
              << scene.imageWidth << "x" << scene.imageHeight << "\n";

    renderScene(scene.camera, scene.triangles, scene.imageWidth, scene.imageHeight,
                scene.backgroundColor, outputPath, scene.lights, scene.materials, scene.textures,
                RenderMode::Shaded, scene.bucketSize);
    std::cout << "Rendered to " << outputPath << "\n";
}

struct SceneJob {
    std::string file;
    RenderMode mode;
};

// Renders every scene in a directory and writes a same-named .ppm for each.
void runSceneBatch(const std::string& scenesDir, const std::string& outDir,
                    const std::vector<SceneJob>& jobs, const std::string& label) {
    for (const SceneJob& job : jobs) {
        std::string inputPath = scenesDir + "/" + job.file;
        SceneData scene = loadScene(inputPath);

        std::string outName = job.file.substr(0, job.file.find(".crtscene")) + ".ppm";
        std::string outputPath = outDir + "/" + outName;

        std::cout << job.file << ": " << scene.triangles.size() << " triangles, "
                  << scene.materials.size() << " materials, "
                  << scene.imageWidth << "x" << scene.imageHeight
                  << (job.mode == RenderMode::Barycentric ? " [barycentric]" : " [shaded]") << "\n";

        renderScene(scene.camera, scene.triangles, scene.imageWidth, scene.imageHeight,
                    scene.backgroundColor, outputPath, scene.lights, scene.materials, scene.textures,
                    job.mode, scene.bucketSize);
        std::cout << "  -> " << outputPath << "\n";
    }
    std::cout << label << ": done\n";
}

// HW09: scene0/1 = barycentric debug view, scene2-5 = shaded (materials + lighting)
void runHw09(const std::string& scenesDir, const std::string& outDir) {
    std::vector<SceneJob> jobs = {
        {"scene0.crtscene", RenderMode::Barycentric},
        {"scene1.crtscene", RenderMode::Barycentric},
        {"scene2.crtscene", RenderMode::Shaded},
        {"scene3.crtscene", RenderMode::Shaded},
        {"scene4.crtscene", RenderMode::Shaded},
        {"scene5.crtscene", RenderMode::Shaded},
    };
    runSceneBatch(scenesDir, outDir, jobs, "HW09 - Shading 01");
}

// HW11: refractive materials, handled inside Renderer.h's shadeHit()
void runHw11(const std::string& scenesDir, const std::string& outDir) {
    std::vector<SceneJob> jobs = {
        {"scene0.crtscene", RenderMode::Shaded},
        {"scene1.crtscene", RenderMode::Shaded},
        {"scene2.crtscene", RenderMode::Shaded},
        {"scene3.crtscene", RenderMode::Shaded},
        {"scene4.crtscene", RenderMode::Shaded},
        {"scene5.crtscene", RenderMode::Shaded},
        {"scene6.crtscene", RenderMode::Shaded},
        {"scene7.crtscene", RenderMode::Shaded},
        {"scene8.crtscene", RenderMode::Shaded},
    };
    runSceneBatch(scenesDir, outDir, jobs, "HW11 - Shading 03");
}

// HW12: textured materials (albedo/edges/checker/bitmap), sampled in Renderer.h's shadeHit()
void runHw12(const std::string& scenesDir, const std::string& outDir) {
    std::vector<SceneJob> jobs = {
        {"scene0.crtscene", RenderMode::Shaded},
        {"scene1.crtscene", RenderMode::Shaded},
        {"scene2.crtscene", RenderMode::Shaded},
        {"scene3.crtscene", RenderMode::Shaded},
        {"scene4.crtscene", RenderMode::Shaded},
    };
    runSceneBatch(scenesDir, outDir, jobs, "HW12 - Textures");
}

// HW13: measures render time with each optimization (AABB, threading, bucket
// size) toggled on/off, so the improvement from each one is visible on its own.
void runHw13(const std::string& scenePath, const std::string& outDir) {
    SceneData scene = loadScene(scenePath);
    std::cout << "Scene: " << scene.triangles.size() << " triangles, "
              << scene.imageWidth << "x" << scene.imageHeight
              << ", bucket_size=" << scene.bucketSize << "\n\n";

    auto timedRender = [&](const std::string& label, const std::string& outName,
                            bool useAABB, int threadCount, int bucketSize) {
        auto start = std::chrono::steady_clock::now();
        renderScene(scene.camera, scene.triangles, scene.imageWidth, scene.imageHeight,
                    scene.backgroundColor, outDir + "/" + outName, scene.lights, scene.materials,
                    scene.textures, RenderMode::Shaded, bucketSize, useAABB, threadCount, /*useBVH=*/false);
        auto end = std::chrono::steady_clock::now();
        double seconds = std::chrono::duration<double>(end - start).count();
        std::cout << label << ": " << seconds << "s\n";
    };

    // Baseline: single thread, no AABB, one big bucket (whole image as one region).
    timedRender("baseline (1 thread, no AABB, single bucket)", "hw13_baseline.ppm",
                false, 1, std::max(scene.imageWidth, scene.imageHeight));

    // + scene AABB only.
    timedRender("+ scene AABB", "hw13_aabb.ppm",
                true, 1, std::max(scene.imageWidth, scene.imageHeight));

    // + multithreaded buckets (using the scene's bucket_size), AABB still on.
    timedRender("+ multithreaded buckets", "hw13_threaded.ppm",
                true, 0, scene.bucketSize);

    std::cout << "\nHW13 - Optimizations 01: done\n";
}

// HW14: measures render time before/after replacing the flat triangle scan
// with a BVH. Uses the previous best config (AABB + multithreading) as the
// "before" baseline, since that's what a real renderer would already have.
void runHw14(const std::string& scenesDir, const std::string& outDir) {
    std::vector<std::string> sceneFiles = { "scene0.crtscene", "scene1.crtscene" };

    for (const std::string& file : sceneFiles) {
        SceneData scene = loadScene(scenesDir + "/" + file);
        std::cout << file << ": " << scene.triangles.size() << " triangles, "
                  << scene.imageWidth << "x" << scene.imageHeight << "\n";

        std::string base = file.substr(0, file.find(".crtscene"));

        auto start = std::chrono::steady_clock::now();
        renderScene(scene.camera, scene.triangles, scene.imageWidth, scene.imageHeight,
                    scene.backgroundColor, outDir + "/" + base + "_before_bvh.ppm",
                    scene.lights, scene.materials, scene.textures, RenderMode::Shaded,
                    scene.bucketSize, /*useAABB=*/true, /*threadCount=*/0, /*useBVH=*/false);
        auto mid = std::chrono::steady_clock::now();
        renderScene(scene.camera, scene.triangles, scene.imageWidth, scene.imageHeight,
                    scene.backgroundColor, outDir + "/" + base + "_after_bvh.ppm",
                    scene.lights, scene.materials, scene.textures, RenderMode::Shaded,
                    scene.bucketSize, /*useAABB=*/true, /*threadCount=*/0, /*useBVH=*/true);
        auto end = std::chrono::steady_clock::now();

        double beforeSeconds = std::chrono::duration<double>(mid - start).count();
        double afterSeconds = std::chrono::duration<double>(end - mid).count();
        std::cout << "  before (flat AABB scan): " << beforeSeconds << "s\n";
        std::cout << "  after  (BVH):            " << afterSeconds << "s\n\n";
    }
    std::cout << "HW14 - Optimizations 02: done\n";
}

void printUsage() {
    std::cout << "Usage:\n"
              << "  ChaosRayTracer rays\n"
              << "  ChaosRayTracer triangle\n"
              << "  ChaosRayTracer intersect\n"
              << "  ChaosRayTracer camera-demo\n"
              << "  ChaosRayTracer scene <input.crtscene> <output.ppm>\n"
              << "  ChaosRayTracer hw09 <scenes_dir> <output_dir>\n"
              << "  ChaosRayTracer hw11 <scenes_dir> <output_dir>\n"
              << "  ChaosRayTracer hw12 <scenes_dir> <output_dir>\n"
              << "  ChaosRayTracer hw13 <scene.crtscene> <output_dir>\n"
              << "  ChaosRayTracer hw14 <scenes_dir> <output_dir>\n";
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
    } else if (command == "hw09") {
        if (argc < 4) {
            std::cout << "hw09 command requires <scenes_dir> <output_dir>\n";
            return 1;
        }
        runHw09(argv[2], argv[3]);
    } else if (command == "hw11") {
        if (argc < 4) {
            std::cout << "hw11 command requires <scenes_dir> <output_dir>\n";
            return 1;
        }
        runHw11(argv[2], argv[3]);
    } else if (command == "hw12") {
        if (argc < 4) {
            std::cout << "hw12 command requires <scenes_dir> <output_dir>\n";
            return 1;
        }
        runHw12(argv[2], argv[3]);
    } else if (command == "hw13") {
        if (argc < 4) {
            std::cout << "hw13 command requires <scene.crtscene> <output_dir>\n";
            return 1;
        }
        runHw13(argv[2], argv[3]);
    } else if (command == "hw14") {
        if (argc < 4) {
            std::cout << "hw14 command requires <scenes_dir> <output_dir>\n";
            return 1;
        }
        runHw14(argv[2], argv[3]);
    } else {
        printUsage();
        return 1;
    }

    return 0;
}
