#ifndef MODEL_H
#define MODEL_H

#include "tgaimage.h"
#include "vec.h"
#include "mat.h"
#include "utils.h"
#include <vector>
#include <string_view>
#include <filesystem>
#include <cmath>
#include <array>

using Face  = std::vector<Vec3>;  // For each vertex, we have <vertex index, texture coordinate, normal index>

class Model {
public:
    // The constructor (throws std::runtime exception)
    Model(std::filesystem::path filePath) noexcept(false);

    // helper function to get the vertex of a face
    Vec3 getVertex(int faceIndex, int index) const {
        return m_vertices[m_faces[faceIndex][index][0]];
    }

    void render(TGAImage& framebuffer, std::vector<double>& zbuffer);
    void rasterize(std::array<Vec4, 3>& clip, TGAImage& framebuffer, std::vector<double>& zbuffer, const TGAColor& color) const;

private:
    // Private member functions to parse model file
    ParseError parseVertex(std::string_view line);
    ParseError parseFace(std::string_view line);
    ParseError parseNormal(std::string_view line);
    ParseError parseTexture(std::string_view line);
    ParseError parseParameter(std::string_view line);

    // Functions to populate transformation matrices
    void viewport(const int x, const int y, const int w, const int h);
    void perspective(const double camera);
    void lookat(const Vec3& eye, const Vec3& center, const Vec3& up);

    // Storing the components of the model
    std::vector<Face> m_faces      {};
    std::vector<Vec3> m_vertices   {};
    std::vector<Vec3> m_normals    {};
    std::vector<Vec3> m_textures   {};
    std::vector<Vec3> m_parameters {};

    // Transformation matrices
    Mat4 m_viewport    {};
    Mat4 m_modelview   {};
    Mat4 m_perspective {};
};

#endif