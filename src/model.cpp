#include "model.h"
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <string>
#include <charconv>
#include <cmath>

constexpr std::string_view whitespaces = " \t\r";

const char* parseErrorMessage(ParseError error) {
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

void Model::render(TGAImage& framebuffer, TGAImage& zbuffer) const {
    for (std::size_t i = 0; i < m_faces.size(); i++) {
        TGAColor color;
        for (int c = 0; c < 3; c++) color[c] = std::rand() % 255;
        for (std::size_t j = 0; j < m_faces[i].size()-2; j++) {
            auto [ax, ay, az] = project(getVertex(i, j)  , framebuffer.height(), framebuffer.width());
            auto [bx, by, bz] = project(getVertex(i, j+1), framebuffer.height(), framebuffer.width());
            auto [cx, cy, cz] = project(getVertex(i, j+2), framebuffer.height(), framebuffer.width());
            drawTriangle(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer, zbuffer, color);
        }
    }
}

void drawLine(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    bool steep = std::abs(bx - ax) < std::abs(by - ay);
    if (steep) {
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax > bx) {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    const int dx = bx - ax;
    const int dy = std::abs(by - ay);
    const int increment_y = (by >= ay) ? 1 : -1;

    int error = 2 * dy - dx;
    int y = ay;
    for (int x = ax; x <= bx; x++) {
        (steep) ? framebuffer.set(y, x, color) : framebuffer.set(x, y, color);

        if (error > 0) {
            y += increment_y;
            error += 2 * (dy - dx);
        } else
            error += 2 * dy;
    }
}

double signedTriangleArea(int ax, int ay, int bx, int by, int cx, int cy) {
    return 0.5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

void drawTriangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage& framebuffer, TGAImage& zbuffer, TGAColor color) {
    int bbminx = std::min(ax, std::min(bx, cx));
    int bbminy = std::min(ay, std::min(by, cy));
    int bbmaxx = std::max(ax, std::max(bx, cx));
    int bbmaxy = std::max(ay, std::max(by, cy));
    double totalArea = signedTriangleArea(ax, ay, bx, by, cx, cy);
    if (totalArea < 1.0) return;    // discard triangles that are facing back (< 0) and area smaller than a pixel (< 1)
    double inverseTotalArea = 1.0 / totalArea;

#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; x++) {
        for (int y = bbminy; y <= bbmaxy; y++) {
            double alpha = signedTriangleArea(x, y, bx, by, cx, cy) * inverseTotalArea;
            double beta  = signedTriangleArea(x, y, cx, cy, ax, ay) * inverseTotalArea;
            double gamma = signedTriangleArea(x, y, ax, ay, bx, by) * inverseTotalArea;
            if (alpha >= 0 && beta >= 0 && gamma >= 0) {
                unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
                if (z > zbuffer.get(x, y)[0]) {
                    zbuffer.set(x, y, {z});
                    framebuffer.set(x, y, color);
                }
            }
        }
    }
}