#include "Bullet.hpp"
#include "Config.hpp"

Bullet::Bullet(Vector2 position, float speed)
    : position_(position), speed_(speed), radius_(cfg::BulletRadius) {}

void Bullet::Update(float dt) {
    position_.y -= speed_ * dt;
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
    return position_.y + radius_ < -20.0f;
}
