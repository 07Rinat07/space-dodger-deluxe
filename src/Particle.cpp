#include "Particle.hpp"

Particle::Particle(Vector2 position, Vector2 velocity, float life, Color color)
    : position_(position), velocity_(velocity), life_(life), maxLife_(life), color_(color) {}

void Particle::Update(float dt) {
    position_.x += velocity_.x * dt;
    position_.y += velocity_.y * dt;
    life_ -= dt;
}

void Particle::Draw() const {
    const float t = life_ / maxLife_;
    const float radius = 2.0f + t * 5.0f;
    DrawCircleV(position_, radius, Fade(color_, t));
}

bool Particle::IsDead() const {
    return life_ <= 0.0f;
}
