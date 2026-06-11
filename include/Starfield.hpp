#pragma once

#include "raylib.h"
#include <vector>

struct Star {
    Vector2 position{};
    float speed{};
    float radius{};
};

class Starfield {
public:
    explicit Starfield(int count = 120);

    void Update(float dt);
    void Draw() const;

private:
    std::vector<Star> stars_;
};
