#pragma once

#include "RaylibCompat.hpp"

class EnemyProjectile {
public:
    EnemyProjectile(Vector2 position, Vector2 velocity, float radius);

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
