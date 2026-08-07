#pragma once
#include <fstream>
#include <string>
#include <stdexcept>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "CRTVector.h"
#include "CRTMatrix.h"
#include "CRTColor.h"
#include "Triangle.h"
#include "Camera.h"
#include "Light.h"

struct SceneData {
    int imageWidth = 1920;
    int imageHeight = 1080;
    CRTColor backgroundColor;
    Camera camera;
    std::vector<CRTTriangle> triangles;
    std::vector<Light> lights;
};

inline CRTColor triangleColorFromIndex(size_t index) {
    static const CRTColor palette[] = {
        CRTColor(220, 60, 60),
        CRTColor(60, 220, 60),
        CRTColor(60, 60, 220),
        CRTColor(220, 220, 60),
        CRTColor(220, 60, 220),
        CRTColor(60, 220, 220)
    };
    return palette[index % 6];
}

inline CRTVector albedoFromIndex(size_t index) {
    static const CRTVector palette[] = {
        CRTVector(0.9f, 0.2f, 0.2f),
        CRTVector(0.2f, 0.9f, 0.2f),
        CRTVector(0.2f, 0.2f, 0.9f),
        CRTVector(0.9f, 0.9f, 0.2f),
        CRTVector(0.9f, 0.2f, 0.9f),
        CRTVector(0.2f, 0.9f, 0.9f),
        CRTVector(0.85f, 0.85f, 0.85f)
    };
    return palette[index % 7];
}

inline SceneData loadScene(const std::string& path) {
    SceneData scene;

    std::ifstream fileStream(path);
    if (!fileStream.is_open()) {
        throw std::runtime_error("Could not open scene file: " + path);
    }

    rapidjson::IStreamWrapper isw(fileStream);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (doc.HasParseError()) {
        throw std::runtime_error("Failed to parse JSON in: " + path);
    }

    if (doc.HasMember("settings")) {
        const auto& settings = doc["settings"];

        if (settings.HasMember("background_color")) {
            const auto& bg = settings["background_color"];
            scene.backgroundColor = CRTColor(
                static_cast<int>(bg[0].GetFloat() * 255),
                static_cast<int>(bg[1].GetFloat() * 255),
                static_cast<int>(bg[2].GetFloat() * 255)
            );
        }

        if (settings.HasMember("image_settings")) {
            const auto& imgSettings = settings["image_settings"];
            scene.imageWidth = imgSettings["width"].GetInt();
            scene.imageHeight = imgSettings["height"].GetInt();
        }
    }

    if (doc.HasMember("camera")) {
        const auto& camera = doc["camera"];

        if (camera.HasMember("position")) {
            const auto& pos = camera["position"];
            scene.camera.position = CRTVector(
                pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat()
            );
        }

        if (camera.HasMember("matrix")) {
            const auto& mat = camera["matrix"];
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    scene.camera.rotation.m[i][j] = mat[i * 3 + j].GetFloat();
                }
            }
        }
    }

    if (doc.HasMember("lights")) {
        const auto& lights = doc["lights"];
        for (rapidjson::SizeType i = 0; i < lights.Size(); ++i) {
            const auto& light = lights[i];
            const auto& pos = light["position"];
            float intensity = light["intensity"].GetFloat();
            scene.lights.push_back(Light(
                CRTVector(pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat()),
                intensity
            ));
        }
    }

    if (doc.HasMember("objects")) {
        const auto& objects = doc["objects"];
        for (rapidjson::SizeType objIdx = 0; objIdx < objects.Size(); ++objIdx) {
            const auto& obj = objects[objIdx];

            std::vector<CRTVector> vertices;
            const auto& verts = obj["vertices"];
            for (rapidjson::SizeType i = 0; i < verts.Size(); i += 3) {
                vertices.push_back(CRTVector(
                    verts[i].GetFloat(),
                    verts[i + 1].GetFloat(),
                    verts[i + 2].GetFloat()
                ));
            }

            const auto& tris = obj["triangles"];
            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3) {
                int i0 = tris[i].GetInt();
                int i1 = tris[i + 1].GetInt();
                int i2 = tris[i + 2].GetInt();

                CRTColor color = triangleColorFromIndex(scene.triangles.size());
                CRTVector albedo = albedoFromIndex(scene.triangles.size());
                scene.triangles.push_back(
                    CRTTriangle(vertices[i0], vertices[i1], vertices[i2], color, albedo)
                );
            }
        }
    }

    return scene;
}
