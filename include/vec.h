#ifndef VEC_H
#define VEC_H

#include <cassert>
#include <iostream>
#include <array>

template<std::size_t N>
struct Vec {
    std::array<double, N> data{};
    double& operator[](std::size_t i)       { assert(i < N); return data[i]; }
    double  operator[](std::size_t i) const { assert(i < N); return data[i]; }
};

template<std::size_t N>
std::ostream& operator<<(std::ostream& out, const Vec<N>& v) {
    for (std::size_t i = 0; i < N; i++)
        out << v[i] << " ";
    return out;
}

template<std::size_t N>
Vec<N> operator+(const Vec<N>& a, const Vec<N>& b) {
    Vec<N> result;
    for (std::size_t i = 0; i < N; i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

template<std::size_t N>
Vec<N> operator-(const Vec<N>& a, const Vec<N>& b) {
    Vec<N> result;
    for (std::size_t i = 0; i < N; i++) {
        result[i] = a[i] - b[i];
    }
    return result;
}

template<>
struct Vec<2> {
    double x = 0, y = 0;
    double& operator[](std::size_t i)       { assert(i < 2); return (&x)[i]; }
    double  operator[](std::size_t i) const { assert(i < 2); return (&x)[i]; }
};

template<>
struct Vec<3> {
    double x = 0, y = 0, z = 0;
    double& operator[](std::size_t i)       { assert(i < 3); return (&x)[i]; }
    double  operator[](std::size_t i) const { assert(i < 3); return (&x)[i]; }
};

template<>
struct Vec<4> {
    double x = 0, y = 0, z = 0, w = 0;
    double& operator[](std::size_t i)       { assert(i < 4); return (&x)[i]; }
    double  operator[](std::size_t i) const { assert(i < 4); return (&x)[i]; }
};

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;

#endif