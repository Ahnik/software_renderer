#ifndef MAT_H
#define MAT_H

#include <cassert>
#include <iostream>
#include <array>
#include "vec.h"

template<std::size_t R, std::size_t C>
struct Mat {
    std::array<Vec<C>, R> rows{};
    const Vec<C>& operator[](std::size_t r) const { assert(r < R); return rows[r]; }
    Vec<C>&       operator[](std::size_t r)       { assert(r < R); return rows[r]; }
};

template<std::size_t N>
Mat<N, N> identity() {
    Mat<N, N> mat{};
    for (std::size_t i = 0; i < N; i++) {
        mat[i][i] = 1.0;
    }
    return mat;
}

template<std::size_t M, std::size_t N, std::size_t P>
Mat<M, P> operator*(const Mat<M, N>& a, const Mat<N, P>& b) {
    Mat<M, P> result{};
    for (std::size_t i = 0; i < M; i++) {
        for (std::size_t j = 0; j < P; j++) {
            for (std::size_t k = 0; k < N; k++)
                result[i][j] += a[i][k] * b[k][i];
        }
    }
    return result;
}

template<std::size_t R, std::size_t C>
Vec<R> operator*(const Mat<R, C>& a, const Vec<C>& b) {
    Vec<R> result{};
    for (std::size_t r = 0; r < R; r++) {
        for (std::size_t c = 0; c < C; c++) {
            result[r] += a[r][c] * b[c];
        }
    }
    return result;
}

template<std::size_t R, std::size_t C>
Mat<C, R> transpose(const Mat<R, C>& m) {
    Mat<C, R> result{};
    for (std::size_t r = 0; r < R; r++) {
        for (std::size_t c = 0; c < C; c++) {
            result[c][r] = m[r][c];
        }
    }
    return result;
}

using Mat2 = Mat<2, 2>;
using Mat3 = Mat<3, 3>;
using Mat4 = Mat<4, 4>;

#endif