#pragma once

#include "Asteroid.hpp"
#include "Bullet.hpp"
#include "Particle.hpp"
#include "Pickup.hpp"
#include "Player.hpp"
#include "Starfield.hpp"
#include "Storage.hpp"
#include "WaveSystem.hpp"
#include <random>
#include <string>
#include <vector>

enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver,
    Settings
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
    std::vector<Bullet> bullets_;
    std::vector<Pickup> pickups_;
    std::vector<Particle> particles_;

    std::mt19937 rng_;
    SaveData saveData_;
    int score_ = 0;
    int bonusScore_ = 0;
    int highScore_ = 0;
    float survivedTime_ = 0.0f;
    float asteroidSpawnTimer_ = 0.0f;
    float pickupSpawnTimer_ = 0.0f;
    float difficulty_ = 1.0f;
    float shotCooldown_ = 0.0f;
    int currentWave_ = 1;
    int bossWaveSpawned_ = 0;
    bool exitRequested_ = false;

#ifndef UNIT_TEST
    bool spritesheetReady_ = false;
    Texture2D spritesheet_{};
    bool audioReady_ = false;
    bool externalShotReady_ = false;
    bool externalPickupReady_ = false;
    bool externalExplosionReady_ = false;
    bool externalMusicReady_ = false;
    Sound shotSound_{};
    Sound pickupSound_{};
    Sound explosionSound_{};
    Music backgroundMusic_{};
    AudioStream musicStream_{};
    float musicPhase_ = 0.0f;
    std::vector<short> musicBuffer_;
#endif

    void HandleInput();
    void Update(float dt);
    void Draw() const;

    void StartNewGame();
    void TogglePause();
    void FinishGame();

    void SpawnAsteroid();
    void SpawnBoss();
    void SpawnPickup();
    void FireBullet();
    void SpawnExplosion(Vector2 position, Color color, int count);
    void SaveSettings();

    void UpdatePlaying(float dt);
    void UpdateParticles(float dt);
    void UpdateMusic();
    void CheckCollisions();
    void RemoveDeadObjects();

    void DrawMenu() const;
    void DrawSettings() const;
    void DrawPlaying() const;
    void DrawPaused() const;
    void DrawGameOver() const;
    void DrawHud() const;
    void DrawCenteredText(const std::string& text, int y, int fontSize, Color color) const;

    float RandomFloat(float minValue, float maxValue);
    int RandomInt(int minValue, int maxValue);

#ifndef UNIT_TEST
    void InitializeAssets();
    void ShutdownAssets();
    void DrawSpriteCell(int cellX, int cellY, Vector2 center, float size, float rotation = 0.0f) const;
    bool HasLivingBoss() const;
    void InitializeAudio();
    void ShutdownAudio();
    void PlayShotSound();
    void PlayPickupSound();
    void PlayExplosionSound();
#endif
};
