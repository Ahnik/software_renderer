#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include "tgaimage.h"
#include "vec.h"
#include "model.h"
#include "gl.h"

constexpr TGAColor white  = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green  = {  0, 255,   0, 255};
constexpr TGAColor red    = {  0,   0, 255, 255};
constexpr TGAColor blue   = {255, 128,  64, 255};
constexpr TGAColor yellow = {  0, 200, 255, 255};

constexpr std::size_t width  = 800;
constexpr std::size_t height = 800;

template<std::integral N>
double binpow(double a, N b) {
    if (a == 0.0 || a == 1.0) return a;
    double res = 1;
    while (b > 0) {
        if (b & 1)
            res *= a;
        a *= a;
        b >>= 1;
    }
    return res;
}

struct RandomShader : IShader {
    RandomShader(const Model &m, const gl::Context& g) : model(m), gl(g) {}

    virtual Vec4 vertex(const int faceIndex, const int index) const {
        Vec3 v = model.getVertex(faceIndex, index);
        Vec4 homogeneous {v.x, v.y, v.z, 1.0};
        return gl.transfromToClip(gl.transformToEye(homogeneous));
    }

    virtual std::pair<bool, TGAColor> fragment(const Vec3& bar) const {
        return {false, color};
    }

    const Model& model;
    const gl::Context& gl;
    TGAColor color {};
};

struct PhongShader : IShader {
    PhongShader(const Model& m, const gl::Context& g, const Vec3& l) : model(m), gl(g) {
        Vec4 temp = normalized(gl.transformToEye(Vec4(l.x, l.y, l.z, 0.0)));
        light.x = temp.x, light.y = temp.y, light.z  = temp.z;
    }

    virtual Vec4 vertex(const int faceIndex, const int vertIndex, const int triIndex) {
        Vec3 v = model.getVertex(faceIndex, vertIndex);
        Vec4 homogeneous {v.x, v.y, v.z, 1.0};
        Vec4 eye_coordinates = gl.transformToEye(homogeneous);
        triangle[triIndex].x = eye_coordinates.x, triangle[triIndex].y = eye_coordinates.y, triangle[triIndex].z = eye_coordinates.z;
        return gl.transfromToClip(eye_coordinates);
    }

    virtual std::pair<bool, TGAColor> fragment(const Vec3& bar) const {
        Vec3 normal = normalized(cross(triangle[1] - triangle[0], triangle[2] - triangle[0]));
        Vec3 reflected = 2 * dot(normal, light) * normal - light;
        Vec3 centroid = (triangle[0] + triangle[1] + triangle[2]) / 3;
        Vec3 view = normalized(-centroid);
        TGAColor fragColor = {255, 255, 255, 255};
        double ambient = 0.3;
        double diff = std::max(0.0, dot(normal, light));
        double spec = binpow(std::max(0.0, dot(reflected, view)), 36);
        for (int i = 0; i < 3; i++)
            fragColor[i] *= std::min(1.0, ambient + 0.4 * diff + 0.9 * spec);
        return {false, fragColor};
    }

    const Model& model;
    const gl::Context& gl;
    TGAColor color {};
    Vec3 light {};                      // direction of light in eye coordinates
    std::array<Vec3, 3> triangle {};    // Triangle primitive in eye coordinates
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./renderer <obj file>\n";
        return 1;
    }
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    CameraContext camera {};
    camera.x = width/16, camera.y = height/16, camera.w = width*7/8, camera.h = width*7/8;
    camera.eye    = {0.0, 0.0, 5.0};
    camera.center = {0.0, 0.0, 0.0};
    camera.up     = {0.0, 1.0, 0.0};
    camera.distance = modulus(camera.eye - camera.center);
    Vec3 light {1.0, 1.0, 1.0};

    gl::Context gl(camera, width, height);

    for (int m = 1; m < argc; m++) {
        try {
            std::filesystem::path filePath(argv[m]);
            Model model(filePath);
            PhongShader shader(model, gl, light);
            for (std::size_t i = 0; i < model.getFaceCount(); i++) {
                Triangle clip {};
                for (std::size_t j = 0; j < model.getVertexCount(i)-2; j++) {
                    clip[0] = shader.vertex(i, 0  , 0);
                    clip[1] = shader.vertex(i, j+1, 1);
                    clip[2] = shader.vertex(i, j+2, 2);
                    gl.rasterize(clip, shader, framebuffer);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
            return 1;
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}