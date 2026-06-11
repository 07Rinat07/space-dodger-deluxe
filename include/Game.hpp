#pragma once

#include "Asteroid.hpp"
#include "Bullet.hpp"
#include "EnemyProjectile.hpp"
#include "Particle.hpp"
#include "Pickup.hpp"
#include "Player.hpp"
#include "Starfield.hpp"
#include "Storage.hpp"
#include "WaveSystem.hpp"
#include <array>
#include <random>
#include <string>
#include <vector>

enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver,
    NameEntry,
    Leaderboard,
    Settings
};

enum class MusicTrack {
    Menu,
    Game,
    Boss,
    GameOver,
    Count
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
    std::vector<EnemyProjectile> enemyProjectiles_;
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
    float bossAttackTimer_ = 0.0f;
    int currentWave_ = 1;
    int bossWaveSpawned_ = 0;
    bool exitRequested_ = false;
    bool scoreSubmitted_ = true;
    std::string playerName_ = "PLAYER";
    struct ExplosionVisual {
        Vector2 position{};
        float age = 0.0f;
        float duration = 0.42f;
        float size = 120.0f;
    };
    std::vector<ExplosionVisual> explosionVisuals_;

#ifndef UNIT_TEST
    bool artReady_ = false;
    Texture2D playerTexture_{};
    Texture2D asteroidRockTexture_{};
    Texture2D asteroidFastTexture_{};
    Texture2D asteroidHeavyTexture_{};
    Texture2D bulletTexture_{};
    Texture2D enemyProjectileTexture_{};
    Texture2D pickupScoreTexture_{};
    Texture2D pickupShieldTexture_{};
    Texture2D bossCruiserTexture_{};
    Texture2D bossStrikerTexture_{};
    Texture2D bossCarrierTexture_{};
    Texture2D playerAnimTexture_{};
    Texture2D bossCruiserAnimTexture_{};
    Texture2D bossStrikerAnimTexture_{};
    Texture2D bossCarrierAnimTexture_{};
    Texture2D explosionAnimTexture_{};
    bool animationReady_ = false;
    bool audioReady_ = false;
    bool externalShotReady_ = false;
    bool externalPickupReady_ = false;
    bool externalExplosionReady_ = false;
    std::array<bool, static_cast<std::size_t>(MusicTrack::Count)> externalMusicReady_{};
    Sound shotSound_{};
    Sound pickupSound_{};
    Sound explosionSound_{};
    std::array<Music, static_cast<std::size_t>(MusicTrack::Count)> musicTracks_{};
    MusicTrack currentMusicTrack_ = MusicTrack::Menu;
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
    void SpawnBossProjectile(Vector2 position, Vector2 velocity, float radius, ProjectilePattern pattern = ProjectilePattern::Straight, float amplitude = 0.0f, float frequency = 1.0f);
    void UpdateBossPatterns(float dt);
    void SpawnPickup();
    void FireBullet();
    void SpawnExplosion(Vector2 position, Color color, int count);
    void SaveSettings();
    void SubmitScore();

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
    void DrawNameEntry() const;
    void DrawLeaderboard() const;
    void DrawHud() const;
    void DrawCenteredText(const std::string& text, int y, int fontSize, Color color) const;

    float RandomFloat(float minValue, float maxValue);
    int RandomInt(int minValue, int maxValue);

#ifndef UNIT_TEST
    void InitializeAssets();
    void ShutdownAssets();
    bool LoadTextureIfExists(Texture2D& texture, const char* path);
    void DrawTextureAsset(const Texture2D& texture, Vector2 center, float size, float rotation = 0.0f) const;
    void DrawTextureFrame(const Texture2D& texture, int frameCount, int frameIndex, Vector2 center, float size, float rotation = 0.0f) const;
    bool HasLivingBoss() const;
    void InitializeAudio();
    void ShutdownAudio();
    void LoadMusicTrack(MusicTrack track, const char* path);
    MusicTrack TargetMusicTrack() const;
    void PlayShotSound();
    void PlayPickupSound();
    void PlayExplosionSound();
#endif
};
