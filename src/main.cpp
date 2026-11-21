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
    if (b == 0) return 1.0;
    double res = 1;
    while (b > 0) {
        if (b & 1)
            res *= a;
        a *= a;
        b >>= 1;
    }
    return res;
}

struct PhongShader : IShader {
    PhongShader(const Model& m, const gl::Context& g, const Vec3& l) : model(m), gl(g) {
        light = normalized(gl.transformToEye(Vec4(l.x, l.y, l.z, 0.0)));
    }

    virtual Vec4 vertex(const int faceIndex, const int vertIndex, const int triIndex) {
        // Get the vertex indices in eye coordinates
        // Vec3 v = model.getVertex(faceIndex, vertIndex);
        // Vec4 v_eye = gl.transformToEye(Vec4(v.x, v.y, v.z, 1.0));
        // triangle[0][triIndex] = v_eye.xyz();

        // // Get the vertex normals in eye coordinates
        // Vec3 vn = model.getNormal(faceIndex, vertIndex);
        // triangle[2][triIndex] = normalized(gl.transformNormal(Vec4(vn.x, vn.y, vn.z, 0.0)).xyz());
        // return gl.transformToClip(v_eye);
        varying_uv[triIndex] = model.getTexture(faceIndex, vertIndex);
        Vec4 v_eye = gl.transformToEye(model.getVertex(faceIndex, vertIndex));
        return gl.transformToClip(v_eye);
    }

    virtual std::pair<bool, TGAColor> fragment(const Vec3& bar) const {
        Vec3 uv = bar[0] * varying_uv[0] + bar[1] * varying_uv[1] + bar[2] * varying_uv[2];
        Vec4 normal = normalized(gl.transformNormal(model.getNormal(uv)));
        Vec4 reflected = 2 * dot(normal, light) * normal - light;
        TGAColor fragColor = model.getDiffuse(uv);
        double ambient = 0.4;
        double diffuse = std::max(0.0, dot(normal, light));
        double specular = (0.5 + 2.0 * model.getSpec(uv)[0]/255.0) * binpow(std::max(0.0, reflected.z), 36);
        for (int i = 0; i < 3; i++)
            fragColor[i] = std::min<int>(255, fragColor[i] * (ambient + diffuse + specular));
        return {false, fragColor};
    }

    const Model& model;
    const gl::Context& gl;
    TGAColor color {};
    Vec4 light {};                      // direction of light in eye coordinates
    std::array<Vec3, 3> triangle {};    // Triangle primitve in eye coordinates
    std::array<Vec3, 3> varying_uv {};  // UV coordinates of the points of the triangle
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./renderer <obj file>\n";
        return 1;
    }
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    CameraContext camera {};
    camera.x = width/16, camera.y = height/16, camera.w = width*7/8, camera.h = width*7/8;
    camera.eye    = {-1.0, 0.0, 2.0};
    camera.center = {0.0, 0.0, 0.0};
    camera.up     = {0.0, 1.0, 0.0};
    camera.distance = modulus(camera.eye - camera.center);
    Vec3 light {1.0, 1.0, 1.0};

    gl::Context gl(camera, width, height);

    for (int m = 1; m < argc; m++) {
        try {
            Model model(argv[m]);
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