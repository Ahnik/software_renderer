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

using Face  = std::vector<Vec3>;  // For each vertex, we have <vertex index, texture coordinate, normal index>

class Model {
public:
    // The constructor (throws std::runtime exception)
    Model(std::filesystem::path filePath) noexcept(false);

    // helper function to get the vertex of a face
    Vec3 getVertex(int faceIndex, int index) const {
        return m_vertices[m_faces[faceIndex][index][0]];
    }

    // generic function to render the image
    template<std::size_t R, std::size_t C>
    void render(TGAImage& framebuffer, TGAImage& debugbuffer, Mat<R, C>& zbuffer) const {
        for (std::size_t i = 0; i < m_faces.size(); i++) {
            TGAColor color;
            for (int c = 0; c < 3; c++) color[c] = std::rand() % 255;
            for (std::size_t j = 0; j < m_faces[i].size()-2; j++) {
                auto [ax, ay, az] = project(persp(rotate(getVertex(i, j)  , 30), 3), framebuffer.height(), framebuffer.width());
                auto [bx, by, bz] = project(persp(rotate(getVertex(i, j+1), 30), 3), framebuffer.height(), framebuffer.width());
                auto [cx, cy, cz] = project(persp(rotate(getVertex(i, j+2), 30), 3), framebuffer.height(), framebuffer.width());
                drawTriangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer, debugbuffer, zbuffer, color);
            }
        }
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