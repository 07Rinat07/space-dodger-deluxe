#include "Game.hpp"
#include "Config.hpp"
#include "Storage.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <ctime>
#include <cmath>
#include <sstream>

Game::Game()
    : starfield_(150), rng_(static_cast<unsigned int>(std::time(nullptr))) {
    InitWindow(cfg::ScreenWidth, cfg::ScreenHeight, cfg::WindowTitle);
    SetExitKey(KEY_NULL);
    SetTargetFPS(cfg::TargetFps);
    highScore_ = Storage::LoadHighScore();
}

Game::~Game() {
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        HandleInput();
        Update(dt);
        Draw();
    }
}

void Game::HandleInput() {
    if (IsKeyPressed(KEY_ENTER) && state_ == GameState::Menu) {
        StartNewGame();
    }

    if (IsKeyPressed(KEY_R) && state_ == GameState::GameOver) {
        StartNewGame();
    }

    if (IsKeyPressed(KEY_P) && (state_ == GameState::Playing || state_ == GameState::Paused)) {
        TogglePause();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (state_ == GameState::Playing || state_ == GameState::Paused || state_ == GameState::GameOver) {
            state_ = GameState::Menu;
        } else {
            // raylib will close the window automatically by the default ESC behavior.
        }
    }
}

void Game::Update(float dt) {
    starfield_.Update(dt);
    UpdateParticles(dt);

    if (state_ == GameState::Playing) {
        UpdatePlaying(dt);
    }
}

void Game::Draw() const {
    BeginDrawing();
    ClearBackground({8, 10, 28, 255});

    starfield_.Draw();

    switch (state_) {
        case GameState::Menu:
            DrawMenu();
            break;
        case GameState::Playing:
            DrawPlaying();
            break;
        case GameState::Paused:
            DrawPlaying();
            DrawPaused();
            break;
        case GameState::GameOver:
            DrawPlaying();
            DrawGameOver();
            break;
    }

    EndDrawing();
}

void Game::StartNewGame() {
    state_ = GameState::Playing;
    player_.Reset();
    asteroids_.clear();
    pickups_.clear();
    particles_.clear();
    score_ = 0;
    bonusScore_ = 0;
    survivedTime_ = 0.0f;
    difficulty_ = 1.0f;
    asteroidSpawnTimer_ = 0.0f;
    pickupSpawnTimer_ = 3.0f;
}

void Game::TogglePause() {
    if (state_ == GameState::Playing) {
        state_ = GameState::Paused;
    } else if (state_ == GameState::Paused) {
        state_ = GameState::Playing;
    }
}

void Game::FinishGame() {
    state_ = GameState::GameOver;
    SpawnExplosion(player_.GetPosition(), RED, 42);

    if (score_ > highScore_) {
        highScore_ = score_;
        Storage::SaveHighScore(highScore_);
    }
}

void Game::SpawnAsteroid() {
    const float radius = RandomFloat(cfg::AsteroidMinRadius, cfg::AsteroidMaxRadius);
    const float x = RandomFloat(radius, cfg::ScreenWidth - radius);
    const float y = -radius - RandomFloat(0.0f, 80.0f);

    const float horizontalDrift = RandomFloat(-80.0f, 80.0f);
    const float verticalSpeed = cfg::AsteroidBaseSpeed * difficulty_ + RandomFloat(0.0f, 120.0f);
    const float rotationSpeed = RandomFloat(-2.5f, 2.5f);

    asteroids_.emplace_back(Vector2{x, y}, Vector2{horizontalDrift, verticalSpeed}, radius, rotationSpeed);
}

void Game::SpawnPickup() {
    const float x = RandomFloat(40.0f, cfg::ScreenWidth - 40.0f);
    const PickupType type = (RandomInt(0, 100) < 72) ? PickupType::Score : PickupType::Shield;
    pickups_.emplace_back(Vector2{x, -25.0f}, type);
}

void Game::SpawnExplosion(Vector2 position, Color color, int count) {
    for (int i = 0; i < count; ++i) {
        const float angle = RandomFloat(0.0f, 2.0f * PI);
        const float speed = RandomFloat(80.0f, 420.0f);
        particles_.emplace_back(
            position,
            Vector2{std::cos(angle) * speed, std::sin(angle) * speed},
            RandomFloat(0.35f, 0.9f),
            color
        );
    }
}

void Game::UpdatePlaying(float dt) {
    survivedTime_ += dt;
    difficulty_ = 1.0f + survivedTime_ / 45.0f;

    // Score consists of two parts:
    // 1) survival points, calculated from total survived time;
    // 2) bonus points, received from pickups and shield collisions.
    score_ = static_cast<int>(survivedTime_ * 10.0f) + bonusScore_;

    player_.Update(dt);

    asteroidSpawnTimer_ -= dt;
    if (asteroidSpawnTimer_ <= 0.0f) {
        SpawnAsteroid();
        const float dynamicDelay = std::max(cfg::MinSpawnDelay, cfg::InitialSpawnDelay - survivedTime_ * 0.008f);
        asteroidSpawnTimer_ = dynamicDelay;
    }

    pickupSpawnTimer_ -= dt;
    if (pickupSpawnTimer_ <= 0.0f) {
        SpawnPickup();
        pickupSpawnTimer_ = RandomFloat(4.5f, 7.0f);
    }

    for (Asteroid& asteroid : asteroids_) {
        asteroid.Update(dt);
    }

    for (Pickup& pickup : pickups_) {
        pickup.Update(dt);
    }

    CheckCollisions();
    RemoveDeadObjects();
}

void Game::UpdateParticles(float dt) {
    for (Particle& particle : particles_) {
        particle.Update(dt);
    }

    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(), [](const Particle& particle) {
            return particle.IsDead();
        }),
        particles_.end()
    );
}

void Game::CheckCollisions() {
    for (auto asteroidIt = asteroids_.begin(); asteroidIt != asteroids_.end(); ++asteroidIt) {
        if (CirclesCollide(player_.GetPosition(), player_.GetRadius(), asteroidIt->GetPosition(), asteroidIt->GetRadius() * 0.82f)) {
            if (player_.HasShield()) {
                // Shield saves the player and destroys the asteroid that caused the collision.
                player_.ActivateShield(0.0f);
                SpawnExplosion(asteroidIt->GetPosition(), SKYBLUE, 28);
                bonusScore_ += 25;
                asteroids_.erase(asteroidIt);
            } else {
                FinishGame();
            }
            return;
        }
    }

    pickups_.erase(
        std::remove_if(pickups_.begin(), pickups_.end(), [this](const Pickup& pickup) {
            if (CirclesCollide(player_.GetPosition(), player_.GetRadius(), pickup.GetPosition(), pickup.GetRadius())) {
                if (pickup.GetType() == PickupType::Score) {
                    bonusScore_ += 100;
                    SpawnExplosion(pickup.GetPosition(), GOLD, 16);
                } else {
                    player_.ActivateShield(5.0f);
                    SpawnExplosion(pickup.GetPosition(), BLUE, 22);
                }
                return true;
            }
            return false;
        }),
        pickups_.end()
    );
}

void Game::RemoveDeadObjects() {
    asteroids_.erase(
        std::remove_if(asteroids_.begin(), asteroids_.end(), [](const Asteroid& asteroid) {
            return asteroid.IsOffScreen();
        }),
        asteroids_.end()
    );

    pickups_.erase(
        std::remove_if(pickups_.begin(), pickups_.end(), [](const Pickup& pickup) {
            return pickup.IsOffScreen();
        }),
        pickups_.end()
    );
}

void Game::DrawMenu() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.25f));
    DrawCenteredText("SPACE DODGER DELUXE", 150, 46, SKYBLUE);
    DrawCenteredText("C++ / raylib arcade pet project", 210, 22, RAYWHITE);

    DrawCenteredText("ENTER - start game", 315, 28, GREEN);
    DrawCenteredText("WASD / ARROWS - move ship", 360, 24, LIGHTGRAY);
    DrawCenteredText("P - pause", 395, 24, LIGHTGRAY);
    DrawCenteredText("ESC - menu / exit", 430, 24, LIGHTGRAY);

    std::ostringstream stream;
    stream << "High score: " << highScore_;
    DrawCenteredText(stream.str(), 510, 24, GOLD);
}

void Game::DrawPlaying() const {
    for (const Pickup& pickup : pickups_) {
        pickup.Draw();
    }

    for (const Asteroid& asteroid : asteroids_) {
        asteroid.Draw();
    }

    for (const Particle& particle : particles_) {
        particle.Draw();
    }

    player_.Draw();
    DrawHud();
}

void Game::DrawPaused() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.55f));
    DrawCenteredText("PAUSED", 280, 48, YELLOW);
    DrawCenteredText("Press P to continue", 345, 26, RAYWHITE);
    DrawCenteredText("Press ESC to return to menu", 385, 22, LIGHTGRAY);
}

void Game::DrawGameOver() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.62f));
    DrawCenteredText("GAME OVER", 230, 54, RED);

    std::ostringstream scoreText;
    scoreText << "Score: " << score_;
    DrawCenteredText(scoreText.str(), 310, 30, RAYWHITE);

    std::ostringstream highText;
    highText << "High score: " << highScore_;
    DrawCenteredText(highText.str(), 350, 28, GOLD);

    DrawCenteredText("R - restart", 430, 28, GREEN);
    DrawCenteredText("ESC - menu", 470, 24, LIGHTGRAY);
}

void Game::DrawHud() const {
    DrawRectangle(14, 12, 260, 82, Fade(BLACK, 0.42f));
    DrawRectangleLines(14, 12, 260, 82, Fade(RAYWHITE, 0.35f));

    DrawText(TextFormat("Score: %d", score_), 28, 24, 22, RAYWHITE);
    DrawText(TextFormat("Best:  %d", highScore_), 28, 52, 20, GOLD);
    DrawText(TextFormat("Time:  %.1f", survivedTime_), 150, 52, 20, LIGHTGRAY);

    if (player_.HasShield()) {
        DrawText("SHIELD ACTIVE", cfg::ScreenWidth - 215, 24, 22, SKYBLUE);
    }
}

void Game::DrawCenteredText(const std::string& text, int y, int fontSize, Color color) const {
    const int width = MeasureText(text.c_str(), fontSize);
    DrawText(text.c_str(), cfg::ScreenWidth / 2 - width / 2, y, fontSize, color);
}

float Game::RandomFloat(float minValue, float maxValue) {
    std::uniform_real_distribution<float> distribution(minValue, maxValue);
    return distribution(rng_);
}

int Game::RandomInt(int minValue, int maxValue) {
    std::uniform_int_distribution<int> distribution(minValue, maxValue);
    return distribution(rng_);
}
