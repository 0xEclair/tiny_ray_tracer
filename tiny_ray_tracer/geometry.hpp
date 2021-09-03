#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

namespace geometry {
    template<size_t DIM>
    struct vec {
        constexpr float& operator[](const size_t i) {
            assert(i < DIM);
            return data[i];
        }
        constexpr const float& operator[](const size_t i) const {
            assert(i < DIM);
            return data[i];
        }
        constexpr size_t size() const {
            return DIM;
        }

        float data[DIM] = {};
    };

    template<size_t DIM>
    vec<DIM> operator*(const vec<DIM>& lhs, const float rhs) {
        vec<DIM> ret;
        for (size_t i = DIM; i--; ret[i] = lhs[i] * rhs);
        return ret;
    }

    template<size_t DIM>
    float operator*(const vec<DIM>& lhs, const vec<DIM>& rhs) {
        float ret = 0;
        for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
        return ret;
    }

    template<size_t DIM>
    vec<DIM> operator+(vec<DIM> lhs, const vec<DIM>& rhs) {
        for (size_t i = DIM; i--; lhs[i] += rhs[i]);
        return lhs;
    }

    template<size_t DIM>
    vec<DIM> operator-(vec<DIM> lhs, const vec<DIM>& rhs) {
        for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
        return lhs;
    }

    auto& operator+=(auto& lhs, const auto& rhs) {
        for (size_t i = lhs.size(); i--; lhs[i] += rhs[i]);
        return lhs;
    }

    template<size_t DIM>
    vec<DIM> operator/=(vec<DIM>& lhs, const float rhs) {
        for (size_t i = DIM; i--; lhs[i] /= rhs);
        return lhs;
    }

    template<>
    struct vec<3> {
        constexpr float& operator[](const size_t i) {
            assert(i < 3);
            return i == 0 ? x : (i == 1 ? y : z);
        }
        constexpr const float& operator[](const size_t i) const {
            assert(i < 3);
            return i == 0 ? x : (i == 1 ? y : z);
        }
        float norm() {
            return std::sqrt(x * x + y * y + z * z);
        }
        vec<3>& normalize(float l = 1) {
            *this = (*this) * (l / norm());
            return *this;
        }
        constexpr auto size() -> size_t {
            return 3;
        }
        float x = 0, y = 0, z = 0;
    };

    using vec3 = vec<3>;
    using vec4 = vec<4>;

    vec3 cross(vec3 v1, vec3 v2) {
        return {
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        };
    }

    template<size_t DIM>
    std::ostream& operator<<(std::ostream& out, const vec<DIM>& v) {
        for (size_t i = 0; i < DIM; i++) {
            out << v[i] << " ";
        }
        out << '\n';
        return out;
    }
}

namespace gm = geometry;