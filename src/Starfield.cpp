#include "Starfield.hpp"
#include "Config.hpp"
#include "raylib.h"

Starfield::Starfield(int count) {
    stars_.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; ++i) {
        stars_.push_back(Star{
            {static_cast<float>(GetRandomValue(0, cfg::ScreenWidth)), static_cast<float>(GetRandomValue(0, cfg::ScreenHeight))},
            static_cast<float>(GetRandomValue(40, 180)),
            static_cast<float>(GetRandomValue(1, 3))
        });
    }
}

void Starfield::Update(float dt) {
    for (Star& star : stars_) {
        star.position.y += star.speed * dt;

        if (star.position.y > cfg::ScreenHeight) {
            star.position.y = -5.0f;
            star.position.x = static_cast<float>(GetRandomValue(0, cfg::ScreenWidth));
        }
    }
}

void Starfield::Draw() const {
    for (const Star& star : stars_) {
        DrawCircleV(star.position, star.radius, Fade(RAYWHITE, 0.45f));
    }
}
