#pragma once

#include <cmath>

namespace sightline {

struct Vec2 {
    float x = 0;
    float y = 0;
};

constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
constexpr Vec2 operator*(Vec2 v, float s) { return {v.x * s, v.y * s}; }
constexpr Vec2 operator*(float s, Vec2 v) { return v * s; }
constexpr float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

inline float length(Vec2 v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}
    
inline Vec2 normalized(Vec2 v) {
    const float len = length(v);
    if (len == 0) {
        return {};
    }
    return {v.x / len, v.y / len};
}

}
