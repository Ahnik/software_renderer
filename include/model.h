#ifndef MODEL_H
#define MODEL_H

#include "tgaimage.h"
#include "vec.h"
#include <vector>
#include <string_view>
#include <array>

using Face  = std::vector<Vec3>;  // For each vertex, we have <vertex index, texture coordinate, normal index>

enum class ParseError {
    Ok = 0,
    UnexpectedEndOfLine,
    InvalidDouble,
    OutOfRange,
    UnexpectedCharacter,
    MalformedFace,
};

struct CameraContext {
    Vec3 eye, center, up;
    double distance;
    int x, y, w, h;
};

// Global functions
constexpr const char *parseErrorMessage(ParseError error);

class Model {
public:
    // The constructor (throws std::runtime exception)
    Model(const std::string& filename) noexcept(false);

    // helper function to get the vertex of a face
    Vec4 getVertex(const int faceIndex, const int vertIndex) const {
        return m_vertices[m_faces[faceIndex][vertIndex][0]];
    }

    // helper function to get the vector normal of a vertex of a face
    Vec4 getNormal(const int faceIndex, const int vertIndex) const {
        return m_normals[m_faces[faceIndex][vertIndex][2]];
    }

    // helper function to get the normal from UV space
    Vec4 getNormal(const Vec3& uv) const {
        TGAColor color = m_normalmap.get(uv[0] * m_normalmap.width(), uv[1] * m_normalmap.height());
        return Vec4(color[2], color[1], color[0], 0.0) * 2.0/255.0 - Vec4(1.0, 1.0, 1.0, 0.0);
    }

    // helper function to get the texture coordinates
    Vec3 getTexture(const int faceIndex, const int vertIndex) const {
        return m_textures[m_faces[faceIndex][vertIndex][1]];
    }

    // getter functions
    std::size_t getFaceCount() const { return m_faces.size(); }
    std::size_t getVertexCount(const int face) const { return m_faces[face].size(); }

private:
    // Private member functions to parse model file
    ParseError parseVertex(std::string_view line);
    ParseError parseFace(std::string_view line);
    ParseError parseNormal(std::string_view line);
    ParseError parseTexture(std::string_view line);
    ParseError parseParameter(std::string_view line);

    // Storing the components of the model
    std::vector<Face> m_faces      {};
    std::vector<Vec4> m_vertices   {};
    std::vector<Vec4> m_normals    {};
    std::vector<Vec3> m_textures   {};
    std::vector<Vec3> m_parameters {};

    // Normal map texture
    TGAImage m_normalmap {};
};

#endif