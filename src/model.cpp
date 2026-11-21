#include "model.h"
#include "mat.h"
#include <fstream>
#include <stdexcept>
#include <string>
#include <charconv>
#include <iostream>

constexpr std::string_view whitespaces = " \t\r";

// Global functions
constexpr const char* parseErrorMessage(ParseError error) {
    switch (error) {
        case ParseError::Ok:                  return "Success";
        case ParseError::UnexpectedEndOfLine: return "Unexpected end of line while parsing";
        case ParseError::InvalidDouble:       return "Invalid floating-point number format";
        case ParseError::OutOfRange:          return "Value is out of range";
        case ParseError::UnexpectedCharacter: return "Encountered unexpected character";
        case ParseError::MalformedFace:       return "Face does not have enought vertices";
    }
    return "Unknown error";
}

Model::Model(const std::string& filename) {
    std::ifstream file;
    file.open(filename, std::ifstream::in);

    if (!file.is_open())
        throw std::runtime_error("Failed to open model file: " + filename);

    std::string rawLine;
    while (std::getline(file, rawLine)) {
        std::string_view line(rawLine);
        ParseError error {ParseError::Ok};
        if (line.starts_with("v "))
            // vertex
            error = parseVertex(line);
        else if (line.starts_with("f "))    // face
            error = parseFace(line);
        else if (line.starts_with("vn "))   // vertex normal
            error = parseNormal(line);
        else if (line.starts_with("vt "))   // texture coordinates
            error = parseTexture(line);
        else if (line.starts_with("vp "))   // parameter space vertices
            error = parseParameter(line);
        else
            continue;

        if (error != ParseError::Ok)
            throw std::runtime_error(parseErrorMessage(error));
    }
    std::cerr << "Vertices: " << m_vertices.size() << " Faces: " << m_faces.size() << '\n';

    auto loadTexture = [&filename](const std::string& suffix, TGAImage& img) {
        std::size_t dot = filename.find_last_of(".");
        if (dot == std::string::npos) return;
        std::string textureFile = filename.substr(0, dot) + suffix;
        std::cerr << "Texture file " << textureFile << " loading: " << (img.read_tga_file(textureFile.c_str()) ? "OK" : "Failed") << '\n';
    };
    loadTexture("_nm.tga", m_normalmap);
    loadTexture("_diffuse.tga", m_diffusemap);
    loadTexture("_spec.tga", m_specmap);
}

ParseError Model::parseVertex(std::string_view line) {
    line.remove_prefix(2);
    Vec4 vertex {0.0, 0.0, 0.0, 1.0};
    for (int i = 0; i < 4; i++) {
        std::string_view::size_type pos = line.find_first_not_of(whitespaces);
        if (pos == std::string_view::npos) {
            if (i < 3) return ParseError::UnexpectedEndOfLine;
            else break;
        }
        line.remove_prefix(pos);

        auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), vertex[i]);
        if (ec == std::errc())
            line.remove_prefix(ptr - line.data());
        else if (ec == std::errc::invalid_argument)
            return ParseError::InvalidDouble;
        else if (ec == std::errc::result_out_of_range)
            return ParseError::OutOfRange;
    }
    vertex = vertex / vertex.w;
    m_vertices.emplace_back(vertex);
    return ParseError::Ok;
}

ParseError Model::parseFace(std::string_view line) {
    line.remove_prefix(2);
    Face face {};
    int vertexCount = 0;
    while (!line.empty()) {
        Vec3 indices {0, 0, 0};
        // remove whitespaces
        std::string_view::size_type pos = line.find_first_not_of(whitespaces);
        if (pos == std::string_view::npos)
            break;
        line.remove_prefix(pos);

        // extract first line
        auto result = std::from_chars(line.data(), line.data() + line.size(), indices[0]);
        if (result.ec == std::errc())
            line.remove_prefix(result.ptr - line.data());
        else if (result.ec == std::errc::invalid_argument)
            return ParseError::InvalidDouble;
        else if (result.ec == std::errc::result_out_of_range)
            return ParseError::OutOfRange;

        for (int i = 1; i < 3; i++) {
            if (!line.starts_with("/"))
                break;

            line.remove_prefix(1);
            if (line.starts_with("/") && i == 1) continue;

            result = std::from_chars(line.data(), line.data() + line.size(), indices[i]);
            if (result.ec == std::errc())
                line.remove_prefix(result.ptr - line.data());
            else if (result.ec == std::errc::invalid_argument)
                return ParseError::InvalidDouble;
            else if (result.ec == std::errc::result_out_of_range)
                return ParseError::OutOfRange;
        }

        indices[0]--, indices[1]--, indices[2]--;
        face.emplace_back(indices);
        vertexCount++;
    }
    if (vertexCount < 3)
        return ParseError::MalformedFace;

    m_faces.emplace_back(face);
    return ParseError::Ok;
}

ParseError Model::parseNormal(std::string_view line) {
    line.remove_prefix(3);
    Vec4 normal {0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < 3; i++) {
        std::string_view::size_type pos = line.find_first_not_of(whitespaces);
        if (pos == std::string_view::npos)
            return ParseError::UnexpectedEndOfLine;
        line.remove_prefix(pos);

        auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), normal[i]);
        if (ec == std::errc())
            line.remove_prefix(ptr - line.data());
        else if (ec == std::errc::invalid_argument)
            return ParseError::InvalidDouble;
        else if (ec == std::errc::result_out_of_range)
            return ParseError::OutOfRange;
    }
    m_normals.emplace_back(normalized(normal));
    return ParseError::Ok;
}

ParseError Model::parseTexture(std::string_view line) {
    line.remove_prefix(3);
    Vec3 texture {0.0, 0.0, 0.0};
    for (int i = 0; i < 3; i++) {
        std::string_view::size_type pos = line.find_first_not_of(whitespaces);
        if (pos == std::string_view::npos) {
            if (i == 0) return ParseError::UnexpectedEndOfLine;
            else break;
        }
        line.remove_prefix(pos);

        auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), texture[i]);
        if (ec == std::errc())
            line.remove_prefix(ptr - line.data());
        else if (ec == std::errc::invalid_argument)
            return ParseError::InvalidDouble;
        else if (ec == std::errc::result_out_of_range)
            return ParseError::OutOfRange;
    }
    texture.y = 1 - texture.y;
    m_textures.emplace_back(texture);
    return ParseError::Ok;
}

ParseError Model::parseParameter(std::string_view line) {
    line.remove_prefix(3);
    Vec3 parameters {0.0, 0.0, 1.0};
    for (int i = 0; i < 3; i++) {
        std::string_view::size_type pos = line.find_first_not_of(whitespaces);
        if (pos == std::string_view::npos) {
            if (i == 0) return ParseError::UnexpectedEndOfLine;
            else break;
        }
        line.remove_prefix(pos);

        auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), parameters[i]);
        if (ec == std::errc())
            line.remove_prefix(ptr - line.data());
        else if (ec == std::errc::invalid_argument)
            return ParseError::InvalidDouble;
        else if (ec == std::errc::result_out_of_range)
            return ParseError::OutOfRange;
    }
    m_parameters.emplace_back(parameters);
    return ParseError::Ok;
}

// helper function to get the vertex of a face
Vec4 Model::getVertex(const int faceIndex, const int vertIndex) const {
    return m_vertices[m_faces[faceIndex][vertIndex][0]];
}

// helper function to get the vector normal of a vertex of a face
Vec4 Model::getNormal(const int faceIndex, const int vertIndex) const {
    return m_normals[m_faces[faceIndex][vertIndex][2]];
}

// helper function to get the normal from UV space
Vec4 Model::getNormal(const Vec3& uv) const {
    TGAColor color = m_normalmap.get(uv[0] * m_normalmap.width(), uv[1] * m_normalmap.height());
    return Vec4(color[2], color[1], color[0], 0.0) * 2.0/255.0 - Vec4(1.0, 1.0, 1.0, 0.0);
}

// helper function to get the texture coordinates
Vec3 Model::getTexture(const int faceIndex, const int vertIndex) const {
    return m_textures[m_faces[faceIndex][vertIndex][1]];
}

// helper function to get the diffusion color from texture coordinates
TGAColor Model::getDiffuse(const Vec3& uv) const {
    return m_diffusemap.get(uv[0] * m_diffusemap.width(), uv[1] * m_diffusemap.height());
}

// helper function to get the specular weight from texture coordinates
TGAColor Model::getSpec(const Vec3& uv) const {
    return m_specmap.get(uv[0] * m_specmap.width(), uv[1] * m_specmap.height());
}