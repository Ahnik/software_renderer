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

void Model::viewport(const int x, const int y, const int w, const int h) {
    m_viewport[0] = {w/2.0, 0.0  , 0.0, x+w/2.0};
    m_viewport[1] = {0.0  , h/2.0, 0.0, y+h/2.0};
    m_viewport[2] = {0.0  , 0.0  , 1.0, 0.0};
    m_viewport[3] = {0.0  , 0.0  , 0.0, 1.0};
}

void Model::perspective(const double camera) {
    m_perspective[0] = {1.0, 0.0, 0.0        , 0.0};
    m_perspective[1] = {0.0, 1.0, 0.0        , 0.0};
    m_perspective[2] = {0.0, 0.0, 1.0        , 0.0};
    m_perspective[3] = {0.0, 0.0, -1.0/camera, 1.0};
}

void Model::lookat(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 n = normalized(eye - center);
    Vec3 l = normalized(cross(up, n));
    Vec3 m = normalized(cross(n, l));
    Mat4 left, right;
    left[0] = {l.x, l.y, l.z, 0.0};
    left[1] = {m.x, m.y, m.z, 0.0};
    left[2] = {n.x, n.y, n.z, 0.0};
    left[3] = {0.0, 0.0, 0.0, 1.0};
    right[0] = {1.0, 0.0, 0.0, -center.x};
    right[1] = {0.0, 1.0, 0.0, -center.y};
    right[2] = {0.0, 0.0, 1.0, -center.z};
    right[3] = {0.0, 0.0, 0.0, 1.0};

    m_modelview = left * right;
}

void Model::rasterize(std::array<Vec4, 3>& clip, TGAImage& framebuffer, std::vector<double>& zbuffer, const TGAColor& color) const {
    std::array<Vec4, 3> screen{};
    for (std::size_t i = 0; i < 3; i++) {
        clip[i] = clip[i] / clip[i].w;
        screen[i] = m_viewport * clip[i];
    }
    auto [bbminx, bbmaxx] = std::minmax({static_cast<int>(screen[0].x), static_cast<int>(screen[1].x), static_cast<int>(screen[2].x)});
    auto [bbminy, bbmaxy] = std::minmax({static_cast<int>(screen[0].y), static_cast<int>(screen[1].y), static_cast<int>(screen[2].y)});

    Mat3 triangle {Vec3(screen[0].x, screen[0].y, 1.0), Vec3(screen[1].x, screen[1].y, 1.0), Vec3(screen[2].x, screen[2].y, 1.0)};
    if (determinant(triangle) < 1.0) return;        // discard triangles that are back facing or smaller than one pixel in size
    Mat3 inverseTranspose = inverse(transpose(triangle));

#pragma omp parallel for
    for (int x = std::max(0, bbminx); x <= std::min(bbmaxx, framebuffer.width()-1); x++) {
        for (int y = std::max(0, bbminy); y <= std::min(bbmaxy, framebuffer.height()-1); y++) {
            Vec3 barycentric = inverseTranspose * Vec3(static_cast<double>(x), static_cast<double>(y), 1.0);
            if (barycentric.x >= 0 && barycentric.y >= 0 && barycentric.z >= 0) {
                double z = dot(barycentric, {clip[0].z, clip[1].z, clip[2].z});
                if (z > zbuffer[x + y*framebuffer.width()]) {
                    zbuffer[x + y*framebuffer.width()] = z;
                    framebuffer.set(x, y, color);
                }
            }
        }
    }
}

void Model::render(TGAImage& framebuffer, std::vector<double>& zbuffer) {
    const Vec3 eye    {-1, 0, 2};
    const Vec3 center {0, 0, 0};
    const Vec3 up     {0, 1, 0};

    lookat(eye, center, up);
    perspective(modulus(eye-center));
    viewport(framebuffer.width()/16, framebuffer.height()/16, framebuffer.width()*7/8, framebuffer.height()*7/8);
    auto composedTransformation = m_perspective * m_modelview;

    for (std::size_t i = 0; i < m_faces.size(); i++) {
        TGAColor color;
        for (int c = 0; c < 3; c++) color[c] = std::rand() % 255;
        for (std::size_t j = 0; j < m_faces[i].size()-2; j++) {
            std::array<Vec4, 3> clip{};     // get the clip coordinates of the model
            for (std::size_t k = 0; k < 3; k++)
                clip[k] = composedTransformation * Vec4(getVertex(i, j+k).x, getVertex(i, j+k).y, getVertex(i, j+k).z, 1.0);
            rasterize(clip, framebuffer, zbuffer, color);
        }
    }
}