#ifndef UTILS_H
#define UTILS_H

#include "vec.h"
#include "tgaimage.h"
#include "mat.h"
#include <cmath>
#include <cassert>
#include <numbers>

using Pixel = std::tuple<int, int, int>;

enum class ParseError {
    Ok = 0,
    UnexpectedEndOfLine,
    InvalidDouble,
    OutOfRange,
    UnexpectedCharacter,
    MalformedFace,
};

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

inline void drawLine(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
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

constexpr double signedTriangleArea(int ax, int ay, int bx, int by, int cx, int cy) {
    return 0.5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

constexpr Pixel project(const Vec3& vertex, int height, int width) {
    return {
        std::round((vertex.x + 1.0) *  width / 2.0),
        std::round((vertex.y + 1.0) * height / 2.0),
        std::round((vertex.z + 1.0) *  255.0 / 2.0)
    };
}

constexpr Vec3 rotate(const Vec3& vertex, double degrees) {
    double radians = std::numbers::pi * degrees / 180.0;
    Mat3 Ry{};
    double c = std::cos(radians), s = std::sin(radians);
    Ry[0] = {c, 0, s};
    Ry[1] = {0, 1, 0};
    Ry[2] = {-s, 0, c};
    return Ry * vertex;
}

constexpr Vec3 persp(const Vec3& vertex, double camera) {
    assert(camera != 0.0);
    double scale = 1.0 - (vertex.z / camera);
    assert(scale != 0.0);
    return vertex / scale;
}

// Generic functions
template<std::size_t R, std::size_t C>
void drawTriangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage& framebuffer, TGAImage& debugbuffer, Mat<R, C>& zbuffer, TGAColor color) {
    int bbminx = std::max(0, std::min(ax, std::min(bx, cx)));
    int bbminy = std::max(0, std::min(ay, std::min(by, cy)));
    int bbmaxx = std::min(static_cast<int>(C-1), std::max(ax, std::max(bx, cx)));
    int bbmaxy = std::min(static_cast<int>(R-1), std::max(ay, std::max(by, cy)));
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
                double z = alpha * az + beta * bz + gamma * cz;
                if (z > zbuffer[y][x]) {
                    zbuffer[y][x] = z;
                    framebuffer.set(x, y, color);
                    if (z > 255.0 || z < 0.0) z = 255.0;
                    debugbuffer.set(x, y, {static_cast<unsigned char>(z)});
                }
            }
        }
    }
}

#endif