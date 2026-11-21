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

    // helper functions
    Vec4 getVertex(const int faceIndex, const int vertIndex) const;
    Vec4 getNormal(const int faceIndex, const int vertIndex) const;
    Vec4 getNormal(const Vec3& uv) const;
    Vec3 getTexture(const int faceIndex, const int vertIndex) const;
    TGAColor getDiffuse(const Vec3& uv) const;
    TGAColor getSpec(const Vec3& uv) const;

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

    // Image maps
    TGAImage m_normalmap {}, m_diffusemap {}, m_specmap {};
};

#endif