#include <fstream>
#include <cmath>
#include <vector>
#include <limits>
#include <iostream>
#include <sstream>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

struct CRTVector {
    float x, y, z;

    CRTVector() : x(0), y(0), z(0) {}
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    CRTVector operator-(const CRTVector& other) const {
        return CRTVector(x - other.x, y - other.y, z - other.z);
    }

    CRTVector operator+(const CRTVector& other) const {
        return CRTVector(x + other.x, y + other.y, z + other.z);
    }

    CRTVector operator*(float s) const {
        return CRTVector(x * s, y * s, z * s);
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
    }
};

float dot(const CRTVector& a, const CRTVector& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

CRTVector cross(const CRTVector& a, const CRTVector& b) {
    return CRTVector(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

struct CRTColor {
    int r, g, b;
    CRTColor() : r(0), g(0), b(0) {}
    CRTColor(int r, int g, int b) : r(r), g(g), b(b) {}
};

struct Ray {
    CRTVector origin;
    CRTVector direction;
    Ray(const CRTVector& origin, const CRTVector& direction)
        : origin(origin), direction(direction) {}
};

struct CRTMatrix3x3 {
    float m[3][3];

    CRTMatrix3x3() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    CRTVector operator*(const CRTVector& v) const {
        return CRTVector(
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        );
    }
};

struct CRTTriangle {
    CRTVector v0, v1, v2;
    CRTVector normal;
    CRTColor color;

    CRTTriangle(const CRTVector& v0, const CRTVector& v1, const CRTVector& v2, const CRTColor& color)
        : v0(v0), v1(v1), v2(v2), color(color) {
        CRTVector e0 = v1 - v0;
        CRTVector e1 = v2 - v0;
        normal = cross(e0, e1).normalize();
    }
};

bool intersectTriangle(const Ray& ray, const CRTTriangle& tri, float& tOut) {
    float rProj = dot(tri.normal, ray.direction);
    if (std::fabs(rProj) < 1e-6f) {
        return false;
    }

    float rpDist = dot(tri.normal, tri.v0 - ray.origin);
    float t = rpDist / rProj;
    if (t < 0.0f) {
        return false;
    }

    CRTVector p = ray.origin + ray.direction * t;

    CRTVector e0 = tri.v1 - tri.v0;
    CRTVector v0p = p - tri.v0;
    float d0 = dot(tri.normal, cross(e0, v0p));

    CRTVector e1 = tri.v2 - tri.v1;
    CRTVector v1p = p - tri.v1;
    float d1 = dot(tri.normal, cross(e1, v1p));

    CRTVector e2 = tri.v0 - tri.v2;
    CRTVector v2p = p - tri.v2;
    float d2 = dot(tri.normal, cross(e2, v2p));

    bool inside = (d0 >= 0.0f && d1 >= 0.0f && d2 >= 0.0f) ||
                  (d0 <= 0.0f && d1 <= 0.0f && d2 <= 0.0f);

    if (!inside) {
        return false;
    }

    tOut = t;
    return true;
}

struct Scene {
    int imageWidth = 1920;
    int imageHeight = 1080;
    CRTColor backgroundColor;
    CRTVector cameraPosition;
    CRTMatrix3x3 cameraRotation;
    std::vector<CRTTriangle> triangles;
};

CRTColor triangleColorFromIndex(size_t index) {
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

Scene loadScene(const std::string& path) {
    Scene scene;

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
            scene.cameraPosition = CRTVector(
                pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat()
            );
        }

        if (camera.HasMember("matrix")) {
            const auto& mat = camera["matrix"];
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    scene.cameraRotation.m[i][j] = mat[i * 3 + j].GetFloat();
                }
            }
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
                scene.triangles.push_back(
                    CRTTriangle(vertices[i0], vertices[i1], vertices[i2], color)
                );
            }
        }
    }

    return scene;
}

Ray generateCameraRay(const Scene& scene, int x, int y) {
    float fx = static_cast<float>(x) + 0.5f;
    float fy = static_cast<float>(y) + 0.5f;

    fx /= static_cast<float>(scene.imageWidth);
    fy /= static_cast<float>(scene.imageHeight);

    fx = (2.0f * fx) - 1.0f;
    fy = 1.0f - (2.0f * fy);

    fx *= static_cast<float>(scene.imageWidth) / static_cast<float>(scene.imageHeight);

    CRTVector localDir(fx, fy, -1.0f);
    CRTVector worldDir = (scene.cameraRotation * localDir).normalize();
    return Ray(scene.cameraPosition, worldDir);
}

void renderScene(const Scene& scene, const std::string& outputPath) {
    std::ofstream ppmFileStream(outputPath, std::ios::out | std::ios::binary);
    ppmFileStream << "P3\n" << scene.imageWidth << " " << scene.imageHeight << "\n255\n";

    for (int rowIdx = 0; rowIdx < scene.imageHeight; ++rowIdx) {
        for (int colIdx = 0; colIdx < scene.imageWidth; ++colIdx) {
            Ray ray = generateCameraRay(scene, colIdx, rowIdx);

            float closestT = std::numeric_limits<float>::max();
            const CRTTriangle* closestTri = nullptr;

            for (const CRTTriangle& tri : scene.triangles) {
                float t;
                if (intersectTriangle(ray, tri, t)) {
                    if (t < closestT) {
                        closestT = t;
                        closestTri = &tri;
                    }
                }
            }

            CRTColor pixelColor = closestTri ? closestTri->color : scene.backgroundColor;
            ppmFileStream << pixelColor.r << " " << pixelColor.g << " " << pixelColor.b << "\t";
        }
        ppmFileStream << "\n";
    }

    ppmFileStream.close();
}

int main(int argc, char** argv) {
    std::string inputPath = (argc > 1) ? argv[1] : "scene0.crtscene";
    std::string outputPath = (argc > 2) ? argv[2] : "output.ppm";

    Scene scene = loadScene(inputPath);
    std::cout << "Loaded " << scene.triangles.size() << " triangles, "
              << scene.imageWidth << "x" << scene.imageHeight << "\n";

    renderScene(scene, outputPath);
    std::cout << "Rendered to " << outputPath << "\n";

    return 0;
}
