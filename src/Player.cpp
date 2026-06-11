#include "Player.hpp"
#include "Config.hpp"
#include "Utils.hpp"

Player::Player() {
    Reset();
}

void Player::Reset() {
    position_ = {cfg::ScreenWidth / 2.0f, cfg::ScreenHeight - 95.0f};
    radius_ = cfg::PlayerRadius;
    speed_ = cfg::PlayerSpeed;
    shieldTimeLeft_ = 0.0f;
}

void Player::Update(float dt) {
    Vector2 direction{0.0f, 0.0f};

#ifndef UNIT_TEST
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        direction.x -= 1.0f;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        direction.x += 1.0f;
    }
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
        direction.y -= 1.0f;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
        direction.y += 1.0f;
    }
#endif

    // Normalize diagonal movement so W+D is not faster than only W or only D.
    const float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
    }

    position_.x += direction.x * speed_ * dt;
    position_.y += direction.y * speed_ * dt;

    if (shieldTimeLeft_ > 0.0f) {
        shieldTimeLeft_ -= dt;
        if (shieldTimeLeft_ < 0.0f) {
            shieldTimeLeft_ = 0.0f;
        }
    }

    KeepInsideScreen();
}

void Player::Draw() const {
#ifndef UNIT_TEST
    // Ship body: triangle pointing upward.
    const Vector2 nose{position_.x, position_.y - radius_ - 10.0f};
    const Vector2 leftWing{position_.x - radius_ - 8.0f, position_.y + radius_};
    const Vector2 rightWing{position_.x + radius_ + 8.0f, position_.y + radius_};

    DrawTriangle(nose, leftWing, rightWing, SKYBLUE);
    DrawTriangleLines(nose, leftWing, rightWing, WHITE);

    // Engine flame.
    DrawTriangle(
        {position_.x - 9.0f, position_.y + radius_ - 2.0f},
        {position_.x + 9.0f, position_.y + radius_ - 2.0f},
        {position_.x, position_.y + radius_ + 24.0f},
        ORANGE
    );

    // Cockpit.
    DrawCircleV({position_.x, position_.y - 6.0f}, 7.0f, DARKBLUE);
    DrawCircleLines(static_cast<int>(position_.x), static_cast<int>(position_.y - 6.0f), 7.0f, RAYWHITE);

    if (HasShield()) {
        DrawCircleLines(static_cast<int>(position_.x), static_cast<int>(position_.y), radius_ + 18.0f, BLUE);
        DrawCircleLines(static_cast<int>(position_.x), static_cast<int>(position_.y), radius_ + 22.0f, Fade(SKYBLUE, 0.7f));
    }
#endif
}

Vector2 Player::GetPosition() const {
    return position_;
}

float Player::GetRadius() const {
    return radius_;
}

void Player::ActivateShield(float seconds) {
    shieldTimeLeft_ = seconds;
}

bool Player::HasShield() const {
    return shieldTimeLeft_ > 0.0f;
}

void Player::KeepInsideScreen() {
    position_.x = ClampFloat(position_.x, radius_ + 10.0f, cfg::ScreenWidth - radius_ - 10.0f);
    position_.y = ClampFloat(position_.y, radius_ + 10.0f, cfg::ScreenHeight - radius_ - 10.0f);
}
