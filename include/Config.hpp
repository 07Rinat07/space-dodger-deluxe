#pragma once

namespace cfg {
    constexpr int ScreenWidth = 1000;
    constexpr int ScreenHeight = 700;
    constexpr int TargetFps = 60;

    constexpr float PlayerRadius = 22.0f;
    constexpr float PlayerSpeed = 420.0f;

    constexpr float AsteroidMinRadius = 18.0f;
    constexpr float AsteroidMaxRadius = 46.0f;
    constexpr float AsteroidBaseSpeed = 170.0f;

    constexpr float PickupRadius = 16.0f;
    constexpr float InitialSpawnDelay = 0.75f;
    constexpr float MinSpawnDelay = 0.22f;
    constexpr float BulletRadius = 5.0f;
    constexpr float BulletSpeed = 640.0f;
    constexpr float ShotCooldown = 0.22f;

    constexpr const char* WindowTitle = "Space Dodger Deluxe - C++ raylib";
    constexpr const char* SaveFileName = "space_dodger_save.json";
}
