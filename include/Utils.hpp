#pragma once

#include "RaylibCompat.hpp"
#include <algorithm>
#include <cmath>

inline float ClampFloat(float value, float minValue, float maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}

inline float Distance(Vector2 a, Vector2 b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

inline bool CirclesCollide(Vector2 a, float radiusA, Vector2 b, float radiusB) {
    return Distance(a, b) <= radiusA + radiusB;
}
