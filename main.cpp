#include <cmath>
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void plot_line_low(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    if (ax > bx) {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    const int dx = bx - ax;
    const int dy = (by > ay) ? (by - ay) : (ay - by);
    const int increment_y = (by >= ay) ? 1 : -1;

    int p = 2 * dy - dx;
    int y = ay;

    for (int x = ax; x <= bx; x++) {
        framebuffer.set(x, y, color);
        if (p > 0) {
            y += increment_y;
            p += 2 * (dy - dx);
        } else
            p += 2 * dy;
    }
}

void plot_line_high(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    if (ay > by) {
        std::swap(ay, by);
        std::swap(ax, bx);
    }
    const int dy = by - ay;
    const int dx = (bx > ax) ? (bx - ax) : (ax - bx);
    const int increment_x = (bx >= ax) ? 1 : -1;

    int p = 2 * dx - dy;
    int x = ax;

    for (int y = ay; y <= by; y++) {
        framebuffer.set(x, y, color);
        if (p > 0) {
            x += increment_x;
            p += 2 * (dx - dy);
        } else
            p += 2 * dx;
    }
}

void draw_line(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) {
    const int dx = bx - ax;
    const int dy = by - ay;
    if (std::abs(dy) < std::abs(dx)) 
        plot_line_low(ax, ay, bx, by, framebuffer, color);
    else
        plot_line_high(ax, ay, bx, by, framebuffer, color);
}

int main(int argc, char** argv) {
    constexpr int width  = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    draw_line(ax, ay, bx, by, framebuffer, blue);
    draw_line(cx, cy, bx, by, framebuffer, green);
    draw_line(cx, cy, ax, ay, framebuffer, yellow);
    draw_line(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}