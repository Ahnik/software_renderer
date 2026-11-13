#ifndef MODEL_H
#define MODEL_H

#include "tgaimage.h"
#include "vec.h"
#include "mat.h"
#include <vector>
#include <string_view>
#include <filesystem>
#include <numbers>
#include <cmath>
#include <cassert>

using Face  = std::vector<Vec3>;  // For each vertex, we have <vertex index, texture coordinate, normal index>
using Pixel = std::tuple<int, int, int>;

enum class ParseError {
    Ok = 0,
    UnexpectedEndOfLine,
    InvalidDouble,
    OutOfRange,
    UnexpectedCharacter,
    MalformedFace,
};

// Global functions
const char* parseErrorMessage(ParseError error);
void drawLine(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color);
double signedTriangleArea(int ax, int ay, int bx, int by, int cx, int cy);
void drawTriangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage& framebuffer, TGAImage& zbuffer, TGAColor color);

constexpr Pixel project(const Vec3& vertex, int height, int width) {
    return {
        std::round((vertex.x + 1.0) *  width / 2.0),
        std::round((vertex.y + 1.0) * height / 2.0),
        std::round((vertex.z + 1.0) *  255.0 / 2.0)
    };
}

constexpr Vec3 rotate(const Vec3& vertex, double degrees) {
    double radians = std::numbers::pi * degrees / 180.0;
    Mat3 Ry{};
    Ry[0] = {std::cos(radians), 0, std::sin(radians)};
    Ry[1] = {0, 1, 0};
    Ry[2] = {-std::sin(radians), 0, std::cos(radians)};
    return Ry * vertex;
}

constexpr Vec3 persp(const Vec3& vertex, double camera) {
    assert(camera != 0.0);
    double scale = 1.0 - (vertex.z / camera);
    assert(scale != 0.0);
    return vertex / scale;
}

class Model {
public:
    // The constructor (throws std::runtime exception)
    Model(std::filesystem::path filePath) noexcept(false);
    void render(TGAImage& framebuffer, TGAImage& zbuffer) const;
    Vec3 getVertex(int faceIndex, int index) const {
        return m_vertices[m_faces[faceIndex][index][0]];
    }

private:
    // Private member functions
    ParseError parseVertex(std::string_view line);
    ParseError parseFace(std::string_view line);
    ParseError parseNormal(std::string_view line);
    ParseError parseTexture(std::string_view line);
    ParseError parseParameter(std::string_view line);

    // Member variables
    std::vector<Face> m_faces      {};
    std::vector<Vec3> m_vertices   {};
    std::vector<Vec3> m_normals    {};
    std::vector<Vec3> m_textures   {};
    std::vector<Vec3> m_parameters {};
};

#endif