#include "Asteroid.hpp"
#include "Config.hpp"
#include <cmath>

Asteroid::Asteroid(Vector2 position, Vector2 velocity, float radius, float rotationSpeed)
    : position_(position), velocity_(velocity), radius_(radius), rotation_(0.0f), rotationSpeed_(rotationSpeed) {}

void Asteroid::Update(float dt) {
    position_.x += velocity_.x * dt;
    position_.y += velocity_.y * dt;
    rotation_ += rotationSpeed_ * dt;
}

void Asteroid::Draw() const {
    // A procedural asteroid: several points around a circle.
    constexpr int points = 10;
    Vector2 vertices[points]{};

    for (int i = 0; i < points; ++i) {
        const float angle = rotation_ + static_cast<float>(i) * 2.0f * PI / static_cast<float>(points);
        const float deformation = (i % 2 == 0) ? 1.0f : 0.72f;
        vertices[i] = {
            position_.x + std::cos(angle) * radius_ * deformation,
            position_.y + std::sin(angle) * radius_ * deformation
        };
    }

    for (int i = 1; i < points - 1; ++i) {
        DrawTriangle(vertices[0], vertices[i], vertices[i + 1], BROWN);
    }

    for (int i = 0; i < points; ++i) {
        DrawLineV(vertices[i], vertices[(i + 1) % points], DARKBROWN);
    }
}

Vector2 Asteroid::GetPosition() const {
    return position_;
}

float Asteroid::GetRadius() const {
    return radius_;
}

bool Asteroid::IsOffScreen() const {
    return position_.y - radius_ > cfg::ScreenHeight + 50.0f ||
           position_.x < -100.0f ||
           position_.x > cfg::ScreenWidth + 100.0f;
}
