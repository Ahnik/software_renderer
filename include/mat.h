#ifndef MAT_H
#define MAT_H

#include <cassert>
#include <array>
#include "vec.h"

template<std::size_t R, std::size_t C>
struct Mat {
    std::array<Vec<C>, R> rows{};
    constexpr const Vec<C>& operator[](std::size_t r) const { assert(r < R); return rows[r]; }
    constexpr Vec<C>&       operator[](std::size_t r)       { assert(r < R); return rows[r]; }
};

template<std::size_t N>
constexpr Mat<N, N> identity() {
    Mat<N, N> mat{};
    for (std::size_t i = 0; i < N; i++) {
        mat[i][i] = 1.0;
    }
    return mat;
}

template<std::size_t M, std::size_t N, std::size_t P>
constexpr Mat<M, P> operator*(const Mat<M, N>& a, const Mat<N, P>& b) {
    Mat<M, P> result{};
    for (std::size_t i = 0; i < M; i++) {
        for (std::size_t k = 0; k < N; k++) {
            const double r = a[i][k];
            for (std::size_t j = 0; j < P; j++)
                result[i][j] += r * b[k][j];
        }
    }
    return result;
}

template<std::size_t R, std::size_t C>
constexpr Vec<R> operator*(const Mat<R, C>& a, const Vec<C>& b) {
    Vec<R> result{};
    for (std::size_t r = 0; r < R; r++) {
        for (std::size_t c = 0; c < C; c++) {
            result[r] += a[r][c] * b[c];
        }
    }
    return result;
}

template<std::size_t R, std::size_t C, Numeric T>
constexpr Mat<R, C> operator*(const Mat<R, C>& a, T n) {
    Mat<R, C> result{};
    for (std::size_t r = 0; r < R; r++) {
        for (std::size_t c = 0; c < C; c++) {
            result[r][c] = a[r][c] * n;
        }
    }
    return result;
}

template<std::size_t R, std::size_t C, Numeric T>
constexpr Mat<R, C> operator*(T n, const Mat<R, C>& a) {
    Mat<R, C> result{};
    for (std::size_t r = 0; r < R; r++) {
        for (std::size_t c = 0; c < C; c++) {
            result[r][c] = a[r][c] * n;
        }
    }
    return result;
}

template<std::size_t R, std::size_t C, Numeric T>
constexpr Mat<R, C> operator/(const Mat<R, C>& m, T n) {
    double inv_n = 1.0 / n;
    return m * inv_n;
}

template<std::size_t R, std::size_t C>
constexpr Mat<C, R> transpose(const Mat<R, C>& m) {
    Mat<C, R> result{};
    for (std::size_t r = 0; r < R; r++) {
        for (std::size_t c = 0; c < C; c++) {
            result[c][r] = m[r][c];
        }
    }
    return result;
}

template<std::size_t N>
constexpr double determinant(const Mat<N, N>& m) {
    double det = 1.0;

    for (std::size_t j = 0; j < N; j++) {
        std::size_t index = j;
        for (std::size_t i = j; i < N; i++) {
            if (m[i][j] > m[index][j])
                index = j;
        }

        if (index != j) {
            for (std::size_t k = 0; k < N; k++)
                std::swap(m[index][k], m[j][k]);
            det *= -1;
        }

        if (std::abs(m[j][j]) < 1e-12)
            return 0.0;

        for (std::size_t i = j+1; i < N; i++) {
            double mult = -m[i][j] / m[j][j];
            for (std::size_t k = 0; k < N; k++)
                m[i][k] += mult * m[j][k];
        }
    }

    for (std::size_t i = 0; i < N; i++)
        det *= m[i][i];

    return det;
}

using Mat2 = Mat<2, 2>;
using Mat3 = Mat<3, 3>;
using Mat4 = Mat<4, 4>;

constexpr Mat2 adjoint(const Mat2& m) {
    Mat2 result{};
    result[0] = { m[1][1], -m[0][1]};
    result[1] = {-m[1][0],  m[0][0]};
    return result;
}

constexpr double determinant(const Mat2& m) {
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

constexpr Mat2 inverse(const Mat2& m) {
    double det = determinant(m);
    assert(det != 0.0);

    double inv_det = 1.0 / det;
    return adjoint(m) * inv_det;
}

constexpr Mat3 adjoint(const Mat3& m) {
    Mat3 result{};
    result[0] = {m[1][1] * m[2][2] - m[1][2] * m[2][1], m[0][2] * m[2][1] - m[0][1] * m[2][2], m[0][1] * m[1][2] - m[0][2] * m[1][1]};
    result[1] = {m[1][2] * m[2][0] - m[1][0] * m[2][2], m[0][0] * m[2][2] - m[0][2] * m[2][0], m[0][2] * m[1][0] - m[0][0] * m[1][2]};
    result[2] = {m[1][0] * m[2][1] - m[1][1] * m[2][0], m[0][1] * m[2][0] - m[0][0] * m[2][1], m[0][0] * m[1][1] - m[0][1] * m[1][0]};
    return result;
}

constexpr double determinant(const Mat3& m) {
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

constexpr Mat3 inverse(const Mat3& m) {
    Mat3 adj = adjoint(m);
    double det = m[0][0] * adj[0][0] + m[0][1] * adj[1][0] + m[0][2] * adj[2][0];
    assert(det != 0.0);

    double inv_det = 1.0 / det;
    return adj * inv_det;
}

#endif