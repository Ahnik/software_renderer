#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include "tgaimage.h"
#include "model.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./main <obj file>\n";
        return 1;
    }
    constexpr int width  = 800;
    constexpr int height = 800;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    try {
        std::filesystem::path filePath(argv[1]);
        Model model(filePath);
        model.renderWireframe(framebuffer, red, white);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}