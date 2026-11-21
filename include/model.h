#ifndef MODEL_H
#define MODEL_H

#include "tgaimage.h"
#include "vec.h"
#include <vector>
#include <string_view>
#include <filesystem>
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
    Model(std::filesystem::path filePath) noexcept(false);

    // helper function to get the vertex of a face
    Vec3 getVertex(const int faceIndex, const int vertIndex) const {
        return m_vertices[m_faces[faceIndex][vertIndex][0]];
    }

    // helper function to get the vector normal of a vertex of a face
    Vec3 getNormal(const int faceIndex, const int vertIndex) const {
        return m_normals[m_faces[faceIndex][vertIndex][2]];
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
    std::vector<Vec3> m_vertices   {};
    std::vector<Vec3> m_normals    {};
    std::vector<Vec3> m_textures   {};
    std::vector<Vec3> m_parameters {};
};

#endif