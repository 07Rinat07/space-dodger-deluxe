#pragma once

#include "RaylibCompat.hpp"

enum class ProjectilePattern {
    Straight,
    Sine,
    Drift,
    Arc
};

class EnemyProjectile {
public:
    EnemyProjectile(Vector2 position, Vector2 velocity, float radius);
    EnemyProjectile(Vector2 position, Vector2 velocity, float radius, ProjectilePattern pattern, float amplitude = 0.0f, float frequency = 1.0f);

    void Update(float dt);
    void Draw() const;

    Vector2 GetPosition() const;
    float GetRadius() const;
    ProjectilePattern GetPattern() const;
    bool HasNearMissAwarded() const;
    void MarkNearMissAwarded();
    bool IsOffScreen() const;

private:
    Vector2 startPosition_{};
    Vector2 position_{};
    Vector2 velocity_{};
    float radius_{};
    ProjectilePattern pattern_ = ProjectilePattern::Straight;
    float age_ = 0.0f;
    float amplitude_ = 0.0f;
    float frequency_ = 1.0f;
    bool nearMissAwarded_ = false;
};
