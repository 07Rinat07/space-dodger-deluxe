#pragma once

#include "RaylibCompat.hpp"

enum class AsteroidType {
    Rock,
    Fast,
    Heavy,
    BossCruiser,
    BossStriker,
    BossCarrier
};

class Asteroid {
public:
    Asteroid(Vector2 position, Vector2 velocity, float radius, float rotationSpeed, AsteroidType type = AsteroidType::Rock);

    void Update(float dt);
    void Draw() const;
    bool TakeDamage(int damage);

    Vector2 GetPosition() const;
    float GetRadius() const;
    AsteroidType GetType() const;
    int GetHealth() const;
    int GetScoreValue() const;
    bool IsOffScreen() const;

private:
    Vector2 position_{};
    Vector2 velocity_{};
    float radius_{};
    float rotation_{};
    float rotationSpeed_{};
    AsteroidType type_{};
    int health_{};
};
