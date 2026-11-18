#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include "tgaimage.h"
#include "model.h"
#include "mat.h"

constexpr TGAColor white  = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green  = {  0, 255,   0, 255};
constexpr TGAColor red    = {  0,   0, 255, 255};
constexpr TGAColor blue   = {255, 128,  64, 255};
constexpr TGAColor yellow = {  0, 200, 255, 255};

constexpr std::size_t width  = 800;
constexpr std::size_t height = 800;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./renderer <obj file>\n";
        return 1;
    }
    TGAImage framebuffer(width, height, TGAImage::RGB);
    // static Mat<height, width> zbuffer{};
    std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

    try {
        std::filesystem::path filePath(argv[1]);
        Model model(filePath);
        model.render(framebuffer, zbuffer);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}