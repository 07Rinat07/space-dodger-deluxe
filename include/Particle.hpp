#pragma once

#include "RaylibCompat.hpp"

class Particle {
public:
    Particle(Vector2 position, Vector2 velocity, float life, Color color);

    void Update(float dt);
    void Draw() const;
    bool IsDead() const;
    Vector2 GetPosition() const;
    float GetLife() const;

private:
    Vector2 position_{};
    Vector2 velocity_{};
    float life_{};
    float maxLife_{};
    Color color_{};
};
