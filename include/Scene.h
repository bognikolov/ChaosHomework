#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "CRTVector.h"
#include "CRTMatrix.h"
#include "CRTColor.h"
#include "Triangle.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"

struct SceneData {
    int imageWidth = 1920;
    int imageHeight = 1080;
    CRTColor backgroundColor;
    Camera camera;
    std::vector<CRTTriangle> triangles;
    std::vector<Light> lights;
    std::vector<Material> materials;
};

// Kept for older homeworks (HW05/HW06 demo scenes built without a materials array).
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

// For triangles whose material has smooth_shading = true, computes per-vertex
// normals by averaging the (area-weighted, via the un-normalized cross product)
// face normals of every triangle sharing that vertex position, then assigns the
// result back onto each triangle's n0/n1/n2. Triangles with flat materials are
// left with their default (flat) vertex normals.
inline void computeSmoothNormals(std::vector<CRTTriangle>& triangles, const std::vector<Material>& materials) {
    // Group identical vertex positions together so shared vertices accumulate
    // contributions from every adjacent triangle, even across different objects.
    struct VecHash {
        size_t operator()(const CRTVector& v) const {
            auto h1 = std::hash<float>{}(v.x);
            auto h2 = std::hash<float>{}(v.y);
            auto h3 = std::hash<float>{}(v.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
    struct VecEq {
        bool operator()(const CRTVector& a, const CRTVector& b) const {
            return a.x == b.x && a.y == b.y && a.z == b.z;
        }
    };

    std::unordered_map<CRTVector, CRTVector, VecHash, VecEq> accumulatedNormals;

    for (const CRTTriangle& tri : triangles) {
        bool smooth = tri.materialIndex >= 0 &&
                      static_cast<size_t>(tri.materialIndex) < materials.size() &&
                      materials[tri.materialIndex].smoothShading;
        if (!smooth) {
            continue;
        }
        // Un-normalized face normal so its magnitude (proportional to area)
        // naturally weights bigger triangles more heavily in the average.
        CRTVector faceNormal = cross(tri.v1 - tri.v0, tri.v2 - tri.v0);
        accumulatedNormals[tri.v0] += faceNormal;
        accumulatedNormals[tri.v1] += faceNormal;
        accumulatedNormals[tri.v2] += faceNormal;
    }

    for (CRTTriangle& tri : triangles) {
        bool smooth = tri.materialIndex >= 0 &&
                      static_cast<size_t>(tri.materialIndex) < materials.size() &&
                      materials[tri.materialIndex].smoothShading;
        if (!smooth) {
            continue;
        }
        tri.n0 = accumulatedNormals[tri.v0].normalize();
        tri.n1 = accumulatedNormals[tri.v1].normalize();
        tri.n2 = accumulatedNormals[tri.v2].normalize();
    }
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
            // The .crtscene format serializes the rotation matrix column-major:
            // mat[i*3+j] is column i, row j. We store CRTMatrix3x3::m[row][col],
            // so we transpose while reading (m[j][i] = mat[i*3+j]).
            for (int col = 0; col < 3; ++col) {
                for (int row = 0; row < 3; ++row) {
                    scene.camera.rotation.m[row][col] = mat[col * 3 + row].GetFloat();
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

    if (doc.HasMember("materials")) {
        const auto& materials = doc["materials"];
        for (rapidjson::SizeType i = 0; i < materials.Size(); ++i) {
            const auto& mat = materials[i];
            Material material;

            if (mat.HasMember("type")) {
                material.type = materialTypeFromString(mat["type"].GetString());
            }
            if (mat.HasMember("albedo")) {
                const auto& albedo = mat["albedo"];
                material.albedo = CRTVector(
                    albedo[0].GetFloat(), albedo[1].GetFloat(), albedo[2].GetFloat()
                );
            }
            if (mat.HasMember("smooth_shading")) {
                material.smoothShading = mat["smooth_shading"].GetBool();
            }

            scene.materials.push_back(material);
        }
    }

    if (doc.HasMember("objects")) {
        const auto& objects = doc["objects"];
        for (rapidjson::SizeType objIdx = 0; objIdx < objects.Size(); ++objIdx) {
            const auto& obj = objects[objIdx];

            int materialIndex = 0;
            if (obj.HasMember("material_index")) {
                materialIndex = obj["material_index"].GetInt();
            }

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

                scene.triangles.push_back(
                    CRTTriangle(vertices[i0], vertices[i1], vertices[i2], materialIndex)
                );
            }
        }
    }

    computeSmoothNormals(scene.triangles, scene.materials);

    return scene;
}
