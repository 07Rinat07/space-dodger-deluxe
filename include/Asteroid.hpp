#pragma once

#include "raylib.h"

class Asteroid {
public:
    Asteroid(Vector2 position, Vector2 velocity, float radius, float rotationSpeed);

    void Update(float dt);
    void Draw() const;

    Vector2 GetPosition() const;
    float GetRadius() const;
    bool IsOffScreen() const;

private:
    Vector2 position_{};
    Vector2 velocity_{};
    float radius_{};
    float rotation_{};
    float rotationSpeed_{};
};
