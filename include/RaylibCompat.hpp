#pragma once

#ifdef UNIT_TEST
struct Vector2 {
    float x;
    float y;
};

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

constexpr float PI = 3.14159265358979323846f;
#else
#include "raylib.h"
#endif
