#pragma once

#include "RaylibCompat.hpp"

class Bullet {
public:
    Bullet(Vector2 position, float speed);
    Bullet(Vector2 position, Vector2 velocity);

    void Update(float dt);
    void Draw() const;

    Vector2 GetPosition() const;
    float GetRadius() const;
    bool IsOffScreen() const;

private:
    Vector2 position_{};
    Vector2 velocity_{};
    float radius_{};
};
