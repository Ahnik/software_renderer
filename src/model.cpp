#include "model.h"
#include "mat.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <string>
#include <charconv>
#include <cmath>

constexpr std::string_view whitespaces = " \t\r";

Model::Model(std::filesystem::path filePath) {
    std::ifstream file(filePath);

    if (!file.is_open())
        throw std::runtime_error("Failed to open model file: " + filePath.string());

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
}

ParseError Model::parseVertex(std::string_view line) {
    line.remove_prefix(2);
    Vec3 vertex {0.0, 0.0, 0.0};
    double w {1.0};
    for (int i = 0; i < 4; i++) {
        std::string_view::size_type pos = line.find_first_not_of(whitespaces);
        if (pos == std::string_view::npos) {
            if (i < 3) return ParseError::UnexpectedEndOfLine;
            else break;
        }
        line.remove_prefix(pos);

        double& value = (i < 3) ? vertex[i] : w;
        auto [ptr, ec] = std::from_chars(line.data(), line.data() + line.size(), value);
        if (ec == std::errc())
            line.remove_prefix(ptr - line.data());
        else if (ec == std::errc::invalid_argument)
            return ParseError::InvalidDouble;
        else if (ec == std::errc::result_out_of_range)
            return ParseError::OutOfRange;
    }
    vertex[0] /= w, vertex[1] /= w, vertex[2] /= w;
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
    Vec3 normal {0.0, 0.0, 0.0};
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
    m_normals.emplace_back(normal);
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