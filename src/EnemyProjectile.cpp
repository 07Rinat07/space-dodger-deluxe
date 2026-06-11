#include "EnemyProjectile.hpp"
#include "Config.hpp"

EnemyProjectile::EnemyProjectile(Vector2 position, Vector2 velocity, float radius)
    : position_(position), velocity_(velocity), radius_(radius) {}

void EnemyProjectile::Update(float dt) {
    position_.x += velocity_.x * dt;
    position_.y += velocity_.y * dt;
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

bool EnemyProjectile::IsOffScreen() const {
    return position_.y - radius_ > cfg::ScreenHeight + 40.0f ||
           position_.x + radius_ < -60.0f ||
           position_.x - radius_ > cfg::ScreenWidth + 60.0f;
}
