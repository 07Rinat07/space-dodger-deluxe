#include "Asteroid.hpp"
#include "Config.hpp"
#include <cmath>

namespace {
int InitialHealth(AsteroidType type) {
    if (type == AsteroidType::Boss) {
        return 14;
    }
    return type == AsteroidType::Heavy ? 2 : 1;
}
}

Asteroid::Asteroid(Vector2 position, Vector2 velocity, float radius, float rotationSpeed, AsteroidType type)
    : position_(position),
      velocity_(velocity),
      radius_(radius),
      rotation_(0.0f),
      rotationSpeed_(rotationSpeed),
      type_(type),
      health_(InitialHealth(type)) {}

void Asteroid::Update(float dt) {
    position_.x += velocity_.x * dt;
    position_.y += velocity_.y * dt;
    rotation_ += rotationSpeed_ * dt;
}

void Asteroid::Draw() const {
#ifndef UNIT_TEST
    const Color fillColor = type_ == AsteroidType::Fast ? MAROON : (type_ == AsteroidType::Heavy ? DARKGRAY : (type_ == AsteroidType::Boss ? PURPLE : BROWN));
    const Color lineColor = type_ == AsteroidType::Fast ? RED : (type_ == AsteroidType::Heavy ? GRAY : (type_ == AsteroidType::Boss ? VIOLET : DARKBROWN));

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
        DrawTriangle(vertices[0], vertices[i], vertices[i + 1], fillColor);
    }

    for (int i = 0; i < points; ++i) {
        DrawLineV(vertices[i], vertices[(i + 1) % points], lineColor);
    }

    if (type_ == AsteroidType::Heavy || type_ == AsteroidType::Boss) {
        DrawCircleLines(static_cast<int>(position_.x), static_cast<int>(position_.y), radius_ * 0.55f, RAYWHITE);
    }
#endif
}

bool Asteroid::TakeDamage(int damage) {
    health_ -= damage;
    return health_ <= 0;
}

Vector2 Asteroid::GetPosition() const {
    return position_;
}

float Asteroid::GetRadius() const {
    return radius_;
}

AsteroidType Asteroid::GetType() const {
    return type_;
}

int Asteroid::GetHealth() const {
    return health_;
}

int Asteroid::GetScoreValue() const {
    switch (type_) {
        case AsteroidType::Fast:
            return 80;
        case AsteroidType::Heavy:
            return 120;
        case AsteroidType::Boss:
            return 900;
        case AsteroidType::Rock:
        default:
            return 55;
    }
}

bool Asteroid::IsOffScreen() const {
    return position_.y - radius_ > cfg::ScreenHeight + 50.0f ||
           position_.x < -100.0f ||
           position_.x > cfg::ScreenWidth + 100.0f;
}
