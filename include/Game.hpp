#pragma once

#include "Asteroid.hpp"
#include "Particle.hpp"
#include "Pickup.hpp"
#include "Player.hpp"
#include "Starfield.hpp"
#include <random>
#include <string>
#include <vector>

enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver
};

class Game {
public:
    Game();
    ~Game();

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void Run();

private:
    GameState state_ = GameState::Menu;
    Player player_;
    Starfield starfield_;
    std::vector<Asteroid> asteroids_;
    std::vector<Pickup> pickups_;
    std::vector<Particle> particles_;

    std::mt19937 rng_;
    int score_ = 0;
    int bonusScore_ = 0;
    int highScore_ = 0;
    float survivedTime_ = 0.0f;
    float asteroidSpawnTimer_ = 0.0f;
    float pickupSpawnTimer_ = 0.0f;
    float difficulty_ = 1.0f;

    void HandleInput();
    void Update(float dt);
    void Draw() const;

    void StartNewGame();
    void TogglePause();
    void FinishGame();

    void SpawnAsteroid();
    void SpawnPickup();
    void SpawnExplosion(Vector2 position, Color color, int count);

    void UpdatePlaying(float dt);
    void UpdateParticles(float dt);
    void CheckCollisions();
    void RemoveDeadObjects();

    void DrawMenu() const;
    void DrawPlaying() const;
    void DrawPaused() const;
    void DrawGameOver() const;
    void DrawHud() const;
    void DrawCenteredText(const std::string& text, int y, int fontSize, Color color) const;

    float RandomFloat(float minValue, float maxValue);
    int RandomInt(int minValue, int maxValue);
};
