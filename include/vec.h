#ifndef VEC_H
#define VEC_H

#include <cassert>
#include <iostream>
#include <array>

template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<std::size_t N>
struct Vec {
    std::array<double, N> data{};
    constexpr double& operator[](std::size_t i)       { assert(i < N); return data[i]; }
    constexpr double  operator[](std::size_t i) const { assert(i < N); return data[i]; }
};

template<std::size_t N>
std::ostream& operator<<(std::ostream& out, const Vec<N>& v) {
    for (std::size_t i = 0; i < N; i++)
        out << v[i] << " ";
    return out;
}

template<std::size_t N>
constexpr Vec<N> operator+(const Vec<N>& a, const Vec<N>& b) {
    Vec<N> result;
    for (std::size_t i = 0; i < N; i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

template<std::size_t N>
constexpr Vec<N> operator-(const Vec<N>& a, const Vec<N>& b) {
    Vec<N> result;
    for (std::size_t i = 0; i < N; i++) {
        result[i] = a[i] - b[i];
    }
    return result;
}

template<std::size_t N>
constexpr double dot(const Vec<N>& a, const Vec<N>& b) {
    double sum = 0.0;
    for (std::size_t i = 0; i < N; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

template<std::size_t N, Numeric T>
constexpr Vec<N> operator*(const Vec<N>& v, T n) {
    Vec<N> result{};
    for (std::size_t i = 0; i < N; i++)
        result[i] = v[i] * n;
    return result;
}

template<std::size_t N, Numeric T>
constexpr Vec<N> operator*(T n, const Vec<N>& v) {
    Vec<N> result{};
    for (std::size_t i = 0; i < N; i++)
        result[i] = v[i] * n;
    return result;
}

template<std::size_t N, Numeric T>
constexpr Vec<N> operator/(const Vec<N>& v, T n) {
    double inv_n = 1.0 / n;
    return v * inv_n;
}

template<>
struct Vec<2> {
    double x = 0, y = 0;
    constexpr double& operator[](std::size_t i)       { assert(i < 2); return i ? y : x; }
    constexpr double  operator[](std::size_t i) const { assert(i < 2); return i ? y : x; }
};

template<>
struct Vec<3> {
    double x = 0, y = 0, z = 0;
    constexpr double& operator[](std::size_t i)       { assert(i < 3); return i ? (i == 1 ? y : z) : x; }
    constexpr double  operator[](std::size_t i) const { assert(i < 3); return i ? (i == 1 ? y : z) : x; }
};

template<>
struct Vec<4> {
    double x = 0, y = 0, z = 0, w = 0;
    constexpr double& operator[](std::size_t i)       { assert(i < 4); return i ? (i == 1 ? y : (i == 2) ? z : w) : x; }
    constexpr double  operator[](std::size_t i) const { assert(i < 4); return i ? (i == 1 ? y : (i == 2) ? z : w) : x; }
};

using Vec2 = Vec<2>;
using Vec3 = Vec<3>;
using Vec4 = Vec<4>;

#endif