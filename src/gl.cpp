#include "gl.h"
#include <limits>
#include <algorithm>

gl::Context::Context(const CameraContext& camera, const int width, const int height) {
    init_zbuffer(width, height);
    lookat(camera.eye, camera.center, camera.up);
    init_perspective(camera.distance);
    init_viewport(camera.x, camera.y, camera.w, camera.h);
}

void gl::Context::lookat(const Vec3& eye, const Vec3& center, const Vec3& up) {
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

void gl::Context::init_viewport(const int x, const int y, const int w, const int h) {
    m_viewport[0] = {w/2.0, 0.0  , 0.0, x+w/2.0};
    m_viewport[1] = {0.0  , h/2.0, 0.0, y+h/2.0};
    m_viewport[2] = {0.0  , 0.0  , 1.0, 0.0};
    m_viewport[3] = {0.0  , 0.0  , 0.0, 1.0};
}

void gl::Context::init_perspective(const double camera) {
    m_perspective[0] = {1.0, 0.0, 0.0        , 0.0};
    m_perspective[1] = {0.0, 1.0, 0.0        , 0.0};
    m_perspective[2] = {0.0, 0.0, 1.0        , 0.0};
    m_perspective[3] = {0.0, 0.0, -1.0/camera, 1.0};
}

void gl::Context::init_zbuffer(const int width, const int height) {
    m_zbuffer = std::vector<double>(width * height, -std::numeric_limits<double>::max());
}

void gl::Context::rasterize(Triangle& clip, const IShader& shader, TGAImage& framebuffer) {
    Triangle screen{};
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
            Vec3 barycentrics = inverseTranspose * Vec3(static_cast<double>(x), static_cast<double>(y), 1.0);
            if (barycentrics.x < 0 || barycentrics.y < 0 || barycentrics.z < 0) continue;
            double z = dot(barycentrics, {clip[0].z, clip[1].z, clip[2].z});
            if (z <= m_zbuffer[x + y*framebuffer.width()]) continue;
            auto [discard, color] = shader.fragment(barycentrics);
            if (discard) continue;
            m_zbuffer[x + y*framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }
}