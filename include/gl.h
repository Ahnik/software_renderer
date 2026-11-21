#ifndef GL_H
#define GL_H

#include "tgaimage.h"
#include "mat.h"
#include "vec.h"
#include "model.h"
#include <vector>
#include <array>

// A triangular primitive is made up of 3 ordered points in homogeneous coordinate system
using Triangle = std::array<Vec4, 3>;

struct IShader {
    virtual ~IShader() = default;
    virtual std::pair<bool, TGAColor> fragment(const Vec3& bar) const = 0;
};

namespace gl {
    class Context {
    public:
        Context(const CameraContext& camera, const int width, const int height);
        void lookat(const Vec3& eye, const Vec3& center, const Vec3& up);
        void init_viewport(const int x, const int y, const int w, const int h);
        void init_perspective(const double camera);
        void init_zbuffer(const int width, const int height);

        // Transformation functions
        Vec4 transformToEye(const Vec4& world)   const { return m_modelview * world; }
        Vec4 transformToClip(const Vec4& eye)    const { return m_perspective * eye; }
        Vec4 transformNormal(const Vec4& normal) const { return inverse(transpose(m_modelview)) * normal; }

        // Function to rasterize a triangular primitive
        void rasterize(Triangle& clip, const IShader& shader, TGAImage& framebuffer);

    private:
        // Z-Buffer
        std::vector<double> m_zbuffer {};

        // Transformation matrices
        Mat4 m_viewport    {};
        Mat4 m_perspective {};
        Mat4 m_modelview   {};
    };
};

#endif