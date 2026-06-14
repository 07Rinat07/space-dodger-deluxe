#include "Bullet.hpp"
#include "Config.hpp"

Bullet::Bullet(Vector2 position, float speed)
    : Bullet(position, Vector2{0.0f, -speed}) {}

Bullet::Bullet(Vector2 position, Vector2 velocity)
    : position_(position), velocity_(velocity), radius_(cfg::BulletRadius) {}

void Bullet::Update(float dt) {
    position_.x += velocity_.x * dt;
    position_.y += velocity_.y * dt;
}

void Bullet::Draw() const {
#ifndef UNIT_TEST
    DrawCircleV(position_, radius_, GOLD);
    DrawCircleV({position_.x, position_.y + radius_ * 0.8f}, radius_ * 0.55f, ORANGE);
#endif
}

Vector2 Bullet::GetPosition() const {
    return position_;
}

float Bullet::GetRadius() const {
    return radius_;
}

bool Bullet::IsOffScreen() const {
    return position_.y + radius_ < -20.0f ||
           position_.x + radius_ < -40.0f ||
           position_.x - radius_ > cfg::ScreenWidth + 40.0f;
}
