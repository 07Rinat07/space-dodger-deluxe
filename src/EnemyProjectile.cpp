#include "EnemyProjectile.hpp"
#include "Config.hpp"
#include <cmath>

EnemyProjectile::EnemyProjectile(Vector2 position, Vector2 velocity, float radius)
    : EnemyProjectile(position, velocity, radius, ProjectilePattern::Straight) {}

EnemyProjectile::EnemyProjectile(Vector2 position, Vector2 velocity, float radius, ProjectilePattern pattern, float amplitude, float frequency)
    : startPosition_(position),
      position_(position),
      velocity_(velocity),
      radius_(radius),
      pattern_(pattern),
      amplitude_(amplitude),
      frequency_(frequency) {}

void EnemyProjectile::Update(float dt) {
    age_ += dt;

    switch (pattern_) {
        case ProjectilePattern::Sine:
            position_.x = startPosition_.x + velocity_.x * age_ + std::sin(age_ * frequency_) * amplitude_;
            position_.y = startPosition_.y + velocity_.y * age_;
            break;
        case ProjectilePattern::Drift:
            velocity_.x += std::sin(age_ * frequency_) * amplitude_ * dt;
            position_.x += velocity_.x * dt;
            position_.y += velocity_.y * dt;
            break;
        case ProjectilePattern::Arc:
            velocity_.y += amplitude_ * dt;
            position_.x += velocity_.x * dt;
            position_.y += velocity_.y * dt;
            break;
        case ProjectilePattern::Straight:
        default:
            position_.x += velocity_.x * dt;
            position_.y += velocity_.y * dt;
            break;
    }
}

void EnemyProjectile::Draw() const {
#ifndef UNIT_TEST
    DrawCircleV(position_, radius_, RED);
    DrawCircleLines(static_cast<int>(position_.x), static_cast<int>(position_.y), radius_ + 2.0f, ORANGE);
#endif
}

Vector2 EnemyProjectile::GetPosition() const {
    return position_;
}

float EnemyProjectile::GetRadius() const {
    return radius_;
}

ProjectilePattern EnemyProjectile::GetPattern() const {
    return pattern_;
}

bool EnemyProjectile::HasNearMissAwarded() const {
    return nearMissAwarded_;
}

void EnemyProjectile::MarkNearMissAwarded() {
    nearMissAwarded_ = true;
}

bool EnemyProjectile::IsOffScreen() const {
    return position_.y - radius_ > cfg::ScreenHeight + 40.0f ||
           position_.x + radius_ < -60.0f ||
           position_.x - radius_ > cfg::ScreenWidth + 60.0f;
}
