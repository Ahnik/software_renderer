#ifndef MODEL_H
#define MODEL_H

#include "tgaimage.h"
#include <vector>
#include <array>
#include <string_view>
#include <filesystem>

using Vertex    = std::array<double, 3>;
using Texture   = std::array<double, 3>;
using Parameter = std::array<double, 3>;
using Normal    = std::array<double, 3>;
using Face      = std::vector<std::array<int, 3>>;

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

class Model {
public:
    Model(std::filesystem::path filePath);
    void renderWireframe(TGAImage& framebuffer, TGAColor lineColor, TGAColor pointColor) const;

private:
    // Private member functions
    ParseError parseVertex(std::string_view line);
    ParseError parseFace(std::string_view line);
    ParseError parseNormal(std::string_view line);
    ParseError parseTexture(std::string_view line);
    ParseError parseParameter(std::string_view line);

    // Member variables
    std::vector<Face>      m_faces      {};
    std::vector<Vertex>    m_vertices   {};
    std::vector<Normal>    m_normals    {};
    std::vector<Texture>   m_textures   {};
    std::vector<Parameter> m_parameters {};
};

#endif