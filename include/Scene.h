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
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct SceneData {
    int imageWidth = 1920;
    int imageHeight = 1080;
    CRTColor backgroundColor;
    Camera camera;
    std::vector<CRTTriangle> triangles;
    std::vector<Light> lights;
    std::vector<Material> materials;
    std::vector<Texture> textures;
};

// For triangles with smooth_shading = true, averages face normals around each
// shared vertex to get smooth per-vertex normals. Flat materials are left alone.
inline void computeSmoothNormals(std::vector<CRTTriangle>& triangles, const std::vector<Material>& materials) {
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
            // matrix is stored column-major, so transpose while reading
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

    if (doc.HasMember("textures")) {
        const auto& textures = doc["textures"];
        for (rapidjson::SizeType i = 0; i < textures.Size(); ++i) {
            const auto& tex = textures[i];
            Texture texture;

            if (tex.HasMember("name")) {
                texture.name = tex["name"].GetString();
            }
            if (tex.HasMember("type")) {
                texture.type = textureTypeFromString(tex["type"].GetString());
            }

            if (texture.type == TextureType::Albedo && tex.HasMember("albedo")) {
                const auto& a = tex["albedo"];
                texture.albedo = CRTVector(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
            }

            if (texture.type == TextureType::Edges) {
                if (tex.HasMember("edge_color")) {
                    const auto& c = tex["edge_color"];
                    texture.edgeColor = CRTVector(c[0].GetFloat(), c[1].GetFloat(), c[2].GetFloat());
                }
                if (tex.HasMember("inner_color")) {
                    const auto& c = tex["inner_color"];
                    texture.innerColor = CRTVector(c[0].GetFloat(), c[1].GetFloat(), c[2].GetFloat());
                }
                if (tex.HasMember("edge_width")) {
                    texture.edgeWidth = tex["edge_width"].GetFloat();
                }
            }

            if (texture.type == TextureType::Checker) {
                if (tex.HasMember("color_A")) {
                    const auto& c = tex["color_A"];
                    texture.colorA = CRTVector(c[0].GetFloat(), c[1].GetFloat(), c[2].GetFloat());
                }
                if (tex.HasMember("color_B")) {
                    const auto& c = tex["color_B"];
                    texture.colorB = CRTVector(c[0].GetFloat(), c[1].GetFloat(), c[2].GetFloat());
                }
                if (tex.HasMember("square_size")) {
                    texture.squareSize = tex["square_size"].GetFloat();
                }
            }

            if (texture.type == TextureType::Bitmap && tex.HasMember("file_path")) {
                std::string filePath = tex["file_path"].GetString();
                // file_path in the scene is like "/textures/dragon.jpg"; resolve it
                // relative to the directory the .crtscene file itself lives in.
                std::string sceneDir;
                size_t slashPos = path.find_last_of("/\\");
                if (slashPos != std::string::npos) {
                    sceneDir = path.substr(0, slashPos);
                }
                std::string resolvedPath = sceneDir + filePath;

                int w, h, ch;
                unsigned char* data = stbi_load(resolvedPath.c_str(), &w, &h, &ch, 3);
                if (data) {
                    texture.width = w;
                    texture.height = h;
                    texture.channels = 3;
                    texture.pixels.assign(data, data + (static_cast<size_t>(w) * h * 3));
                    stbi_image_free(data);
                } else {
                    throw std::runtime_error("Could not load texture image: " + resolvedPath);
                }
            }

            scene.textures.push_back(texture);
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
                if (albedo.IsString()) {
                    // Named reference into the scene's texture list.
                    std::string texName = albedo.GetString();
                    for (size_t t = 0; t < scene.textures.size(); ++t) {
                        if (scene.textures[t].name == texName) {
                            material.textureIndex = static_cast<int>(t);
                            break;
                        }
                    }
                } else {
                    material.albedo = CRTVector(
                        albedo[0].GetFloat(), albedo[1].GetFloat(), albedo[2].GetFloat()
                    );
                }
            }
            if (mat.HasMember("smooth_shading")) {
                material.smoothShading = mat["smooth_shading"].GetBool();
            }
            if (mat.HasMember("ior")) {
                material.ior = mat["ior"].GetFloat();
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

            std::vector<CRTVector> uvs;
            if (obj.HasMember("uvs")) {
                const auto& uvArray = obj["uvs"];
                for (rapidjson::SizeType i = 0; i < uvArray.Size(); i += 3) {
                    uvs.push_back(CRTVector(
                        uvArray[i].GetFloat(),
                        uvArray[i + 1].GetFloat(),
                        uvArray[i + 2].GetFloat()
                    ));
                }
            }

            const auto& tris = obj["triangles"];
            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3) {
                int i0 = tris[i].GetInt();
                int i1 = tris[i + 1].GetInt();
                int i2 = tris[i + 2].GetInt();

                CRTTriangle triangle(vertices[i0], vertices[i1], vertices[i2], materialIndex);
                if (!uvs.empty()) {
                    triangle.uv0 = uvs[i0];
                    triangle.uv1 = uvs[i1];
                    triangle.uv2 = uvs[i2];
                }
                scene.triangles.push_back(triangle);
            }
        }
    }

    computeSmoothNormals(scene.triangles, scene.materials);

    return scene;
}
