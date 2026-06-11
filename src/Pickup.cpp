#include "Pickup.hpp"
#include "Config.hpp"
#include <cmath>

Pickup::Pickup(Vector2 position, PickupType type)
    : position_(position), type_(type), radius_(cfg::PickupRadius), speed_(120.0f), pulse_(0.0f) {}

void Pickup::Update(float dt) {
    position_.y += speed_ * dt;
    pulse_ += dt * 5.0f;
}

void Pickup::Draw() const {
#ifndef UNIT_TEST
    const float animatedRadius = radius_ + std::sin(pulse_) * 3.0f;

    if (type_ == PickupType::Score) {
        DrawPoly(position_, 5, animatedRadius, 18.0f, GOLD);
        DrawPolyLines(position_, 5, animatedRadius + 2.0f, 18.0f, YELLOW);
        DrawText("+", static_cast<int>(position_.x - 7), static_cast<int>(position_.y - 12), 24, DARKBROWN);
    } else {
        DrawCircleV(position_, animatedRadius, Fade(BLUE, 0.75f));
        DrawCircleLines(static_cast<int>(position_.x), static_cast<int>(position_.y), animatedRadius + 3.0f, SKYBLUE);
        DrawText("S", static_cast<int>(position_.x - 7), static_cast<int>(position_.y - 12), 24, RAYWHITE);
    }
#endif
}

Vector2 Pickup::GetPosition() const {
    return position_;
}

float Pickup::GetRadius() const {
    return radius_;
}

PickupType Pickup::GetType() const {
    return type_;
}

bool Pickup::IsOffScreen() const {
    return position_.y - radius_ > cfg::ScreenHeight + 30.0f;
}
