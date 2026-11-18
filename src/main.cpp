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

    const Model &model;
    const gl::Context& gl;
    TGAColor color {};
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./renderer <obj file>\n";
        return 1;
    }
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    // std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());
    CameraContext camera {};
    camera.x = width/16, camera.y = height/16, camera.w = width*7/8, camera.h = width*7/8;
    camera.eye    = {-1.0, 0.0, 2.0};
    camera.center = {0.0, 0.0, 0.0};
    camera.up     = {0.0, 1.0, 0.0};
    camera.distance = modulus(camera.eye - camera.center);

    gl::Context gl(camera, width, height);

    for (int m = 1; m < argc; m++) {
        try {
            std::filesystem::path filePath(argv[m]);
            Model model(filePath);
            RandomShader shader(model, gl);
            for (std::size_t i = 0; i < model.getFaceCount(); i++) {
                for (int c = 0; c < 3; c++) shader.color[c] = std::rand() % 255;
                Triangle clip {};
                for (std::size_t j = 0; j < model.getVertexCount(i)-2; j++) {
                    clip[0] = shader.vertex(i, 0);
                    clip[1] = shader.vertex(i, j+1);
                    clip[2] = shader.vertex(i, j+2);
                }
                gl.rasterize(clip, shader, framebuffer);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << '\n';
            return 1;
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}