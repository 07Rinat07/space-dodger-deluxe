#include "Game.hpp"
#include "Config.hpp"
#include "Storage.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <ctime>
#include <cmath>
#include <vector>
#include <sstream>

#ifndef UNIT_TEST
namespace {
Sound CreateTone(float frequency, float duration, float volume) {
    constexpr int sampleRate = 22050;
    const int frameCount = static_cast<int>(static_cast<float>(sampleRate) * duration);
    std::vector<short> samples(static_cast<std::size_t>(frameCount));

    for (int i = 0; i < frameCount; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        const float envelope = 1.0f - t / duration;
        samples[static_cast<std::size_t>(i)] = static_cast<short>(std::sin(2.0f * PI * frequency * t) * 32000.0f * volume * envelope);
    }

    Wave wave{};
    wave.frameCount = static_cast<unsigned int>(frameCount);
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples.data();
    return LoadSoundFromWave(wave);
}

const char* DifficultyDisplayName(DifficultyLevel difficulty) {
    switch (difficulty) {
        case DifficultyLevel::Easy:
            return "easy";
        case DifficultyLevel::Hard:
            return "hard";
        case DifficultyLevel::Normal:
        default:
            return "normal";
    }
}
}
#endif

Game::Game()
    : starfield_(150), rng_(static_cast<unsigned int>(std::time(nullptr))) {
    InitWindow(cfg::ScreenWidth, cfg::ScreenHeight, cfg::WindowTitle);
    SetExitKey(KEY_NULL);
    SetTargetFPS(cfg::TargetFps);
    saveData_ = Storage::Load();
    highScore_ = saveData_.highScore;
#ifndef UNIT_TEST
    InitializeAssets();
    InitializeAudio();
#endif
}

Game::~Game() {
#ifndef UNIT_TEST
    ShutdownAudio();
    ShutdownAssets();
#endif
    if (IsWindowReady()) {
        CloseWindow();
    }
}

void Game::Run() {
    while (!WindowShouldClose() && !exitRequested_) {
        const float dt = GetFrameTime();
        HandleInput();
        if (exitRequested_) {
            break;
        }
        Update(dt);
        Draw();
    }
}

void Game::HandleInput() {
    if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) && state_ == GameState::Menu) {
        StartNewGame();
    }

    if (IsKeyPressed(KEY_S) && state_ == GameState::Menu) {
        state_ = GameState::Settings;
    }

    if (IsKeyPressed(KEY_L) && state_ == GameState::Menu) {
        state_ = GameState::Leaderboard;
    }

    if (state_ == GameState::NameEntry) {
        int key = GetCharPressed();
        while (key > 0) {
            if (playerName_.size() < 12 && ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z') || key == '_' || key == '-')) {
                playerName_.push_back(static_cast<char>(key));
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !playerName_.empty()) {
            playerName_.pop_back();
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            SubmitScore();
        }
    }

    if (state_ == GameState::Settings) {
        if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
            saveData_.difficulty = NextDifficulty(saveData_.difficulty);
            SaveSettings();
        }
        if (IsKeyPressed(KEY_M)) {
            saveData_.musicEnabled = !saveData_.musicEnabled;
            SaveSettings();
        }
        if (IsKeyPressed(KEY_N)) {
            saveData_.soundEnabled = !saveData_.soundEnabled;
            SaveSettings();
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            state_ = GameState::Menu;
        }
    }

    if ((IsKeyPressed(KEY_R) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) && state_ == GameState::GameOver) {
        StartNewGame();
    }

    if (IsKeyPressed(KEY_P) && (state_ == GameState::Playing || state_ == GameState::Paused)) {
        TogglePause();
    }

    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && state_ == GameState::Playing) {
        FireBullet();
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (state_ == GameState::Menu) {
            exitRequested_ = true;
        } else if (state_ == GameState::NameEntry) {
            SubmitScore();
        } else {
            state_ = GameState::Menu;
        }
    }
}

void Game::Update(float dt) {
    starfield_.Update(dt);
    UpdateParticles(dt);
#ifndef UNIT_TEST
    UpdateMusic();
#endif

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
        case GameState::NameEntry:
            DrawPlaying();
            DrawNameEntry();
            break;
        case GameState::Leaderboard:
            DrawLeaderboard();
            break;
        case GameState::Settings:
            DrawSettings();
            break;
    }

    EndDrawing();
}

void Game::StartNewGame() {
    state_ = GameState::Playing;
    player_.Reset();
    asteroids_.clear();
    bullets_.clear();
    enemyProjectiles_.clear();
    pickups_.clear();
    particles_.clear();
    score_ = 0;
    bonusScore_ = 0;
    survivedTime_ = 0.0f;
    difficulty_ = 1.0f;
    asteroidSpawnTimer_ = 0.0f;
    pickupSpawnTimer_ = 3.0f;
    shotCooldown_ = 0.0f;
    bossAttackTimer_ = 1.8f;
    currentWave_ = 1;
    bossWaveSpawned_ = 0;
    scoreSubmitted_ = true;
}

void Game::TogglePause() {
    if (state_ == GameState::Playing) {
        state_ = GameState::Paused;
    } else if (state_ == GameState::Paused) {
        state_ = GameState::Playing;
    }
}

void Game::FinishGame() {
    state_ = GameState::NameEntry;
    SpawnExplosion(player_.GetPosition(), RED, 42);

    if (score_ > highScore_) {
        highScore_ = score_;
    }
    saveData_.highScore = highScore_;
    scoreSubmitted_ = false;
    playerName_ = SanitizePlayerName(saveData_.leaderboard.empty() ? "PLAYER" : saveData_.leaderboard.front().name);
#ifndef UNIT_TEST
    PlayExplosionSound();
#endif
}

void Game::SpawnAsteroid() {
    const int roll = RandomInt(0, 100);
    const AsteroidType type = roll < 18 ? AsteroidType::Fast : (roll > 82 ? AsteroidType::Heavy : AsteroidType::Rock);
    const float typeRadiusBonus = type == AsteroidType::Heavy ? 16.0f : (type == AsteroidType::Fast ? -6.0f : 0.0f);
    const float radius = std::max(14.0f, RandomFloat(cfg::AsteroidMinRadius, cfg::AsteroidMaxRadius) + typeRadiusBonus);
    const float x = RandomFloat(radius, cfg::ScreenWidth - radius);
    const float y = -radius - RandomFloat(0.0f, 80.0f);

    const float typeSpeed = type == AsteroidType::Fast ? 1.55f : (type == AsteroidType::Heavy ? 0.78f : 1.0f);
    const float horizontalDrift = RandomFloat(-80.0f, 80.0f) * typeSpeed;
    const float verticalSpeed = (cfg::AsteroidBaseSpeed * difficulty_ + RandomFloat(0.0f, 120.0f)) * DifficultySpeedMultiplier(saveData_.difficulty) * typeSpeed;
    const float rotationSpeed = RandomFloat(-2.5f, 2.5f);

    asteroids_.emplace_back(Vector2{x, y}, Vector2{horizontalDrift, verticalSpeed}, radius, rotationSpeed, type);
}

void Game::SpawnBoss() {
    const int bossSelector = (currentWave_ / 3) % 3;
    const AsteroidType bossType = bossSelector == 1 ? AsteroidType::BossStriker : (bossSelector == 2 ? AsteroidType::BossCarrier : AsteroidType::BossCruiser);
    const float radius = bossType == AsteroidType::BossStriker ? 62.0f : (bossType == AsteroidType::BossCarrier ? 86.0f : 74.0f);
    const float x = RandomFloat(radius + 20.0f, cfg::ScreenWidth - radius - 20.0f);
    const float y = -radius;
    const float horizontalDrift = bossType == AsteroidType::BossStriker ? RandomFloat(-135.0f, 135.0f) : RandomFloat(-90.0f, 90.0f);
    const float verticalSpeed = (bossType == AsteroidType::BossCarrier ? 62.0f : 80.0f + currentWave_ * 4.0f) * DifficultySpeedMultiplier(saveData_.difficulty);

    asteroids_.emplace_back(Vector2{x, y}, Vector2{horizontalDrift, verticalSpeed}, radius, 0.45f, bossType);
    bossAttackTimer_ = 1.2f;
}

void Game::SpawnBossProjectile(Vector2 position, Vector2 velocity, float radius) {
    enemyProjectiles_.emplace_back(position, velocity, radius);
}

void Game::UpdateBossPatterns(float dt) {
    bossAttackTimer_ -= dt;
    if (bossAttackTimer_ > 0.0f) {
        return;
    }

    std::vector<Asteroid> spawned;
    for (const Asteroid& asteroid : asteroids_) {
        const AsteroidType type = asteroid.GetType();
        if (type != AsteroidType::BossCruiser && type != AsteroidType::BossStriker && type != AsteroidType::BossCarrier) {
            continue;
        }

        const Vector2 origin = asteroid.GetPosition();
        const float healthRatio = static_cast<float>(asteroid.GetHealth()) / static_cast<float>(std::max(1, asteroid.GetMaxHealth()));
        const bool finalPhase = healthRatio <= 0.35f;
        const bool midPhase = healthRatio <= 0.68f;

        if (type == AsteroidType::BossStriker) {
            spawned.emplace_back(Vector2{origin.x - 34.0f, origin.y + 45.0f}, Vector2{-110.0f, 260.0f}, 18.0f, 2.2f, AsteroidType::Fast);
            spawned.emplace_back(Vector2{origin.x + 34.0f, origin.y + 45.0f}, Vector2{110.0f, 260.0f}, 18.0f, -2.2f, AsteroidType::Fast);
            SpawnBossProjectile(Vector2{origin.x, origin.y + 52.0f}, Vector2{0.0f, finalPhase ? 420.0f : 330.0f}, finalPhase ? 11.0f : 9.0f);
            if (midPhase) {
                SpawnBossProjectile(Vector2{origin.x - 28.0f, origin.y + 48.0f}, Vector2{-140.0f, 320.0f}, 8.0f);
                SpawnBossProjectile(Vector2{origin.x + 28.0f, origin.y + 48.0f}, Vector2{140.0f, 320.0f}, 8.0f);
            }
            bossAttackTimer_ = finalPhase ? 0.62f : 1.0f;
        } else if (type == AsteroidType::BossCarrier) {
            spawned.emplace_back(Vector2{origin.x - 46.0f, origin.y + 60.0f}, Vector2{-55.0f, 190.0f}, 24.0f, 1.0f, AsteroidType::Rock);
            spawned.emplace_back(Vector2{origin.x + 46.0f, origin.y + 60.0f}, Vector2{55.0f, 190.0f}, 24.0f, -1.0f, AsteroidType::Rock);
            spawned.emplace_back(Vector2{origin.x, origin.y + 72.0f}, Vector2{0.0f, 180.0f}, 30.0f, 0.6f, AsteroidType::Heavy);
            if (midPhase) {
                SpawnBossProjectile(Vector2{origin.x - 58.0f, origin.y + 48.0f}, Vector2{-90.0f, 260.0f}, 10.0f);
                SpawnBossProjectile(Vector2{origin.x + 58.0f, origin.y + 48.0f}, Vector2{90.0f, 260.0f}, 10.0f);
            }
            bossAttackTimer_ = finalPhase ? 1.05f : 1.7f;
        } else {
            spawned.emplace_back(Vector2{origin.x, origin.y + 56.0f}, Vector2{0.0f, 240.0f}, 26.0f, 1.8f, AsteroidType::Heavy);
            SpawnBossProjectile(Vector2{origin.x - 34.0f, origin.y + 42.0f}, Vector2{-70.0f, finalPhase ? 360.0f : 285.0f}, 9.0f);
            SpawnBossProjectile(Vector2{origin.x + 34.0f, origin.y + 42.0f}, Vector2{70.0f, finalPhase ? 360.0f : 285.0f}, 9.0f);
            bossAttackTimer_ = finalPhase ? 0.85f : 1.35f;
        }
    }

    asteroids_.insert(asteroids_.end(), spawned.begin(), spawned.end());
}

void Game::SpawnPickup() {
    const float x = RandomFloat(40.0f, cfg::ScreenWidth - 40.0f);
    const PickupType type = (RandomInt(0, 100) < 72) ? PickupType::Score : PickupType::Shield;
    pickups_.emplace_back(Vector2{x, -25.0f}, type);
}

void Game::FireBullet() {
    if (shotCooldown_ > 0.0f) {
        return;
    }

    const Vector2 playerPosition = player_.GetPosition();
    bullets_.emplace_back(Vector2{playerPosition.x, playerPosition.y - player_.GetRadius() - 18.0f}, cfg::BulletSpeed);
    shotCooldown_ = cfg::ShotCooldown;
#ifndef UNIT_TEST
    PlayShotSound();
#endif
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

void Game::SaveSettings() {
    saveData_.highScore = highScore_;
    Storage::Save(saveData_);
}

void Game::SubmitScore() {
    if (!scoreSubmitted_) {
        saveData_.highScore = highScore_;
        saveData_.leaderboard = AddScoreToLeaderboard(saveData_.leaderboard, playerName_, score_);
        Storage::Save(saveData_);
        scoreSubmitted_ = true;
    }
    state_ = GameState::GameOver;
}

void Game::UpdatePlaying(float dt) {
    survivedTime_ += dt;
    const WaveInfo wave = ComputeWaveInfo(survivedTime_);
    currentWave_ = wave.number;
    difficulty_ = (1.0f + survivedTime_ / 45.0f) * wave.enemySpeedMultiplier;

    // Score consists of two parts:
    // 1) survival points, calculated from total survived time;
    // 2) bonus points, received from pickups and shield collisions.
    score_ = static_cast<int>(survivedTime_ * static_cast<float>(DifficultyScoreMultiplier(saveData_.difficulty))) + bonusScore_;

    player_.Update(dt);
    shotCooldown_ = std::max(0.0f, shotCooldown_ - dt);

    if (wave.bossWave && bossWaveSpawned_ != wave.number && !HasLivingBoss()) {
        SpawnBoss();
        bossWaveSpawned_ = wave.number;
    }
    UpdateBossPatterns(dt);

    asteroidSpawnTimer_ -= dt;
    if (asteroidSpawnTimer_ <= 0.0f) {
        SpawnAsteroid();
        const float dynamicDelay = std::max(cfg::MinSpawnDelay, cfg::InitialSpawnDelay - survivedTime_ * 0.008f) *
            DifficultySpawnMultiplier(saveData_.difficulty) * wave.spawnDelayMultiplier;
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

    for (Bullet& bullet : bullets_) {
        bullet.Update(dt);
    }

    for (EnemyProjectile& projectile : enemyProjectiles_) {
        projectile.Update(dt);
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

void Game::UpdateMusic() {
#ifndef UNIT_TEST
    if (!audioReady_) {
        return;
    }

    if (!saveData_.musicEnabled) {
        for (std::size_t i = 0; i < musicTracks_.size(); ++i) {
            if (externalMusicReady_[i] && IsMusicStreamPlaying(musicTracks_[i])) {
                StopMusicStream(musicTracks_[i]);
            }
        }
        if (IsAudioStreamPlaying(musicStream_)) {
            StopAudioStream(musicStream_);
        }
        return;
    }

    const MusicTrack targetTrack = TargetMusicTrack();
    const std::size_t targetIndex = static_cast<std::size_t>(targetTrack);
    if (externalMusicReady_[targetIndex]) {
        if (currentMusicTrack_ != targetTrack) {
            const std::size_t currentIndex = static_cast<std::size_t>(currentMusicTrack_);
            if (externalMusicReady_[currentIndex] && IsMusicStreamPlaying(musicTracks_[currentIndex])) {
                StopMusicStream(musicTracks_[currentIndex]);
            }
            currentMusicTrack_ = targetTrack;
        }
        if (!IsMusicStreamPlaying(musicTracks_[targetIndex])) {
            PlayMusicStream(musicTracks_[targetIndex]);
        }
        UpdateMusicStream(musicTracks_[targetIndex]);
        return;
    }

    if (!IsAudioStreamPlaying(musicStream_)) {
        PlayAudioStream(musicStream_);
    }

    if (IsAudioStreamProcessed(musicStream_)) {
        constexpr int sampleRate = 22050;
        constexpr int frames = 512;
        constexpr float melody[] = {220.0f, 277.18f, 329.63f, 246.94f};
        musicBuffer_.resize(frames);
        for (int i = 0; i < frames; ++i) {
            const int beat = static_cast<int>(musicPhase_ * 2.0f) % 4;
            const float frequency = melody[beat];
            const float tone = std::sin(2.0f * PI * frequency * musicPhase_) * 0.16f;
            const float bass = std::sin(2.0f * PI * (frequency * 0.5f) * musicPhase_) * 0.09f;
            musicBuffer_[static_cast<std::size_t>(i)] = static_cast<short>((tone + bass) * 28000.0f);
            musicPhase_ += 1.0f / static_cast<float>(sampleRate);
        }
        UpdateAudioStream(musicStream_, musicBuffer_.data(), frames);
    }
#endif
}

void Game::CheckCollisions() {
    for (auto bulletIt = bullets_.begin(); bulletIt != bullets_.end();) {
        bool bulletRemoved = false;
        for (auto asteroidIt = asteroids_.begin(); asteroidIt != asteroids_.end(); ++asteroidIt) {
            if (CirclesCollide(bulletIt->GetPosition(), bulletIt->GetRadius(), asteroidIt->GetPosition(), asteroidIt->GetRadius())) {
                SpawnExplosion(bulletIt->GetPosition(), ORANGE, 8);
                if (asteroidIt->TakeDamage(1)) {
                    bonusScore_ += asteroidIt->GetScoreValue();
                    SpawnExplosion(asteroidIt->GetPosition(), GOLD, 18);
                    asteroids_.erase(asteroidIt);
#ifndef UNIT_TEST
                    PlayExplosionSound();
#endif
                }
                bulletIt = bullets_.erase(bulletIt);
                bulletRemoved = true;
                break;
            }
        }

        if (!bulletRemoved) {
            ++bulletIt;
        }
    }

    for (auto asteroidIt = asteroids_.begin(); asteroidIt != asteroids_.end(); ++asteroidIt) {
        if (CirclesCollide(player_.GetPosition(), player_.GetRadius(), asteroidIt->GetPosition(), asteroidIt->GetRadius() * 0.82f)) {
            if (player_.HasShield()) {
                // Shield saves the player and destroys the asteroid that caused the collision.
                player_.ActivateShield(0.0f);
                SpawnExplosion(asteroidIt->GetPosition(), SKYBLUE, 28);
                bonusScore_ += 25;
                asteroids_.erase(asteroidIt);
#ifndef UNIT_TEST
                PlayExplosionSound();
#endif
            } else {
                FinishGame();
            }
            return;
        }
    }

    for (auto projectileIt = enemyProjectiles_.begin(); projectileIt != enemyProjectiles_.end(); ++projectileIt) {
        if (CirclesCollide(player_.GetPosition(), player_.GetRadius(), projectileIt->GetPosition(), projectileIt->GetRadius())) {
            if (player_.HasShield()) {
                player_.ActivateShield(0.0f);
                SpawnExplosion(projectileIt->GetPosition(), SKYBLUE, 14);
                enemyProjectiles_.erase(projectileIt);
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
#ifndef UNIT_TEST
                PlayPickupSound();
#endif
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

    bullets_.erase(
        std::remove_if(bullets_.begin(), bullets_.end(), [](const Bullet& bullet) {
            return bullet.IsOffScreen();
        }),
        bullets_.end()
    );

    enemyProjectiles_.erase(
        std::remove_if(enemyProjectiles_.begin(), enemyProjectiles_.end(), [](const EnemyProjectile& projectile) {
            return projectile.IsOffScreen();
        }),
        enemyProjectiles_.end()
    );
}

void Game::DrawMenu() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.25f));
    DrawCenteredText("SPACE DODGER DELUXE", 150, 46, SKYBLUE);
    DrawCenteredText("arcade with weapons, enemies and procedural audio", 210, 22, RAYWHITE);

    DrawCenteredText("ENTER - start game", 315, 28, GREEN);
    DrawCenteredText("L - leaderboard   S - settings", 350, 24, SKYBLUE);
    DrawCenteredText("WASD / ARROWS - move, SPACE / CTRL - shoot", 390, 24, LIGHTGRAY);
    DrawCenteredText("P - pause, ESC - menu / exit", 430, 24, LIGHTGRAY);

    std::ostringstream stream;
    stream << "High score: " << highScore_ << "   Difficulty: " << DifficultyToString(saveData_.difficulty);
    DrawCenteredText(stream.str(), 510, 24, GOLD);
}

void Game::DrawSettings() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.35f));
    DrawCenteredText("SETTINGS", 150, 46, SKYBLUE);

    std::ostringstream difficultyText;
    difficultyText << "D / RIGHT - difficulty: " << DifficultyToString(saveData_.difficulty);
    DrawCenteredText(difficultyText.str(), 275, 26, RAYWHITE);
    DrawCenteredText(saveData_.musicEnabled ? "M - music: on" : "M - music: off", 325, 24, saveData_.musicEnabled ? GREEN : RED);
    DrawCenteredText(saveData_.soundEnabled ? "N - sounds: on" : "N - sounds: off", 365, 24, saveData_.soundEnabled ? GREEN : RED);
    DrawCenteredText("ENTER or ESC - back", 455, 24, LIGHTGRAY);
}

void Game::DrawPlaying() const {
    for (const Pickup& pickup : pickups_) {
        if (artReady_) {
            DrawTextureAsset(pickup.GetType() == PickupType::Score ? pickupScoreTexture_ : pickupShieldTexture_, pickup.GetPosition(), pickup.GetRadius() * 3.0f);
        } else {
            pickup.Draw();
        }
    }

    for (const Asteroid& asteroid : asteroids_) {
        if (artReady_) {
            const Texture2D* texture = &asteroidRockTexture_;
            if (asteroid.GetType() == AsteroidType::Fast) {
                texture = &asteroidFastTexture_;
            } else if (asteroid.GetType() == AsteroidType::Heavy) {
                texture = &asteroidHeavyTexture_;
            } else if (asteroid.GetType() == AsteroidType::BossCruiser) {
                texture = &bossCruiserTexture_;
            } else if (asteroid.GetType() == AsteroidType::BossStriker) {
                texture = &bossStrikerTexture_;
            } else if (asteroid.GetType() == AsteroidType::BossCarrier) {
                texture = &bossCarrierTexture_;
            }
            DrawTextureAsset(*texture, asteroid.GetPosition(), asteroid.GetRadius() * 2.25f);
        } else {
            asteroid.Draw();
        }
    }

    for (const Bullet& bullet : bullets_) {
        if (artReady_) {
            DrawTextureAsset(bulletTexture_, bullet.GetPosition(), bullet.GetRadius() * 4.0f);
        } else {
            bullet.Draw();
        }
    }

    for (const EnemyProjectile& projectile : enemyProjectiles_) {
        if (artReady_) {
            DrawTextureAsset(enemyProjectileTexture_, projectile.GetPosition(), projectile.GetRadius() * 3.4f);
        } else {
            projectile.Draw();
        }
    }

    for (const Particle& particle : particles_) {
        particle.Draw();
    }

    if (artReady_) {
        DrawTextureAsset(playerTexture_, player_.GetPosition(), player_.GetRadius() * 3.2f);
        if (player_.HasShield()) {
            DrawCircleLines(static_cast<int>(player_.GetPosition().x), static_cast<int>(player_.GetPosition().y), player_.GetRadius() + 22.0f, SKYBLUE);
        }
    } else {
        player_.Draw();
    }
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

    int y = 395;
    DrawCenteredText("Leaderboard", y, 22, SKYBLUE);
    y += 30;
    for (std::size_t i = 0; i < saveData_.leaderboard.size() && i < 5; ++i) {
        std::ostringstream row;
        row << i + 1 << ". " << saveData_.leaderboard[i].name << "  " << saveData_.leaderboard[i].score;
        DrawCenteredText(row.str(), y, 20, LIGHTGRAY);
        y += 24;
    }

    DrawCenteredText("R / ENTER / SPACE - restart", 575, 26, GREEN);
    DrawCenteredText("ESC - menu", 615, 22, LIGHTGRAY);
}

void Game::DrawNameEntry() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.68f));
    DrawCenteredText("NEW SCORE", 190, 46, GOLD);

    std::ostringstream scoreText;
    scoreText << "Score: " << score_;
    DrawCenteredText(scoreText.str(), 265, 30, RAYWHITE);

    std::ostringstream nameText;
    nameText << "Name: " << (playerName_.empty() ? "_" : playerName_);
    DrawCenteredText(nameText.str(), 340, 30, SKYBLUE);
    DrawCenteredText("Type letters or numbers, BACKSPACE edits", 410, 22, LIGHTGRAY);
    DrawCenteredText("ENTER saves score", 445, 24, GREEN);
}

void Game::DrawLeaderboard() const {
    DrawRectangle(0, 0, cfg::ScreenWidth, cfg::ScreenHeight, Fade(BLACK, 0.35f));
    DrawCenteredText("LEADERBOARD", 120, 46, GOLD);

    int y = 210;
    if (saveData_.leaderboard.empty()) {
        DrawCenteredText("No scores yet", y, 26, LIGHTGRAY);
    } else {
        for (std::size_t i = 0; i < saveData_.leaderboard.size() && i < 5; ++i) {
            std::ostringstream row;
            row << i + 1 << ". " << saveData_.leaderboard[i].name << "  " << saveData_.leaderboard[i].score;
            DrawCenteredText(row.str(), y, 28, i == 0 ? SKYBLUE : RAYWHITE);
            y += 48;
        }
    }

    DrawCenteredText("ESC - back", 560, 24, LIGHTGRAY);
}

void Game::DrawHud() const {
    DrawRectangle(14, 12, 260, 82, Fade(BLACK, 0.42f));
    DrawRectangleLines(14, 12, 260, 82, Fade(RAYWHITE, 0.35f));

    DrawText(TextFormat("Score: %d", score_), 28, 24, 22, RAYWHITE);
    DrawText(TextFormat("Best:  %d", highScore_), 28, 52, 20, GOLD);
    DrawText(TextFormat("Time:  %.1f", survivedTime_), 150, 52, 20, LIGHTGRAY);
    DrawText(TextFormat("Wave:  %d", currentWave_), 150, 76, 18, SKYBLUE);

    if (player_.HasShield()) {
        DrawText("SHIELD ACTIVE", cfg::ScreenWidth - 215, 24, 22, SKYBLUE);
    }

    DrawText(TextFormat("Difficulty: %s", DifficultyDisplayName(saveData_.difficulty)), cfg::ScreenWidth - 210, 56, 18, LIGHTGRAY);
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

#ifndef UNIT_TEST
void Game::InitializeAssets() {
    const bool playerLoaded = LoadTextureIfExists(playerTexture_, "assets/textures/sprites/player_ship.png");
    const bool rockLoaded = LoadTextureIfExists(asteroidRockTexture_, "assets/textures/sprites/asteroid_rock.png");
    const bool fastLoaded = LoadTextureIfExists(asteroidFastTexture_, "assets/textures/sprites/asteroid_fast.png");
    const bool heavyLoaded = LoadTextureIfExists(asteroidHeavyTexture_, "assets/textures/sprites/asteroid_heavy.png");
    const bool bulletLoaded = LoadTextureIfExists(bulletTexture_, "assets/textures/sprites/bullet.png");
    const bool enemyProjectileLoaded = LoadTextureIfExists(enemyProjectileTexture_, "assets/textures/sprites/enemy_projectile.png");
    const bool scoreLoaded = LoadTextureIfExists(pickupScoreTexture_, "assets/textures/sprites/pickup_score.png");
    const bool shieldLoaded = LoadTextureIfExists(pickupShieldTexture_, "assets/textures/sprites/pickup_shield.png");
    const bool bossLoaded = LoadTextureIfExists(bossCruiserTexture_, "assets/textures/sprites/boss_cruiser.png");
    const bool bossStrikerLoaded = LoadTextureIfExists(bossStrikerTexture_, "assets/textures/sprites/boss_striker.png");
    const bool bossCarrierLoaded = LoadTextureIfExists(bossCarrierTexture_, "assets/textures/sprites/boss_carrier.png");
    artReady_ = playerLoaded && rockLoaded && fastLoaded && heavyLoaded && bulletLoaded && enemyProjectileLoaded &&
        scoreLoaded && shieldLoaded && bossLoaded && bossStrikerLoaded && bossCarrierLoaded;
}

void Game::ShutdownAssets() {
    if (playerTexture_.id != 0) UnloadTexture(playerTexture_);
    if (asteroidRockTexture_.id != 0) UnloadTexture(asteroidRockTexture_);
    if (asteroidFastTexture_.id != 0) UnloadTexture(asteroidFastTexture_);
    if (asteroidHeavyTexture_.id != 0) UnloadTexture(asteroidHeavyTexture_);
    if (bulletTexture_.id != 0) UnloadTexture(bulletTexture_);
    if (enemyProjectileTexture_.id != 0) UnloadTexture(enemyProjectileTexture_);
    if (pickupScoreTexture_.id != 0) UnloadTexture(pickupScoreTexture_);
    if (pickupShieldTexture_.id != 0) UnloadTexture(pickupShieldTexture_);
    if (bossCruiserTexture_.id != 0) UnloadTexture(bossCruiserTexture_);
    if (bossStrikerTexture_.id != 0) UnloadTexture(bossStrikerTexture_);
    if (bossCarrierTexture_.id != 0) UnloadTexture(bossCarrierTexture_);
    artReady_ = false;
}

bool Game::LoadTextureIfExists(Texture2D& texture, const char* path) {
    if (!FileExists(path)) {
        return false;
    }
    texture = LoadTexture(path);
    return texture.id != 0;
}

void Game::DrawTextureAsset(const Texture2D& texture, Vector2 center, float size, float rotation) const {
    const Rectangle source{0.0f, 0.0f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
    const Rectangle destination{
        center.x,
        center.y,
        size,
        size
    };
    DrawTexturePro(texture, source, destination, {size / 2.0f, size / 2.0f}, rotation, WHITE);
}

bool Game::HasLivingBoss() const {
    return std::any_of(asteroids_.begin(), asteroids_.end(), [](const Asteroid& asteroid) {
        return asteroid.GetType() == AsteroidType::BossCruiser ||
               asteroid.GetType() == AsteroidType::BossStriker ||
               asteroid.GetType() == AsteroidType::BossCarrier;
    });
}

void Game::InitializeAudio() {
    InitAudioDevice();
    audioReady_ = IsAudioDeviceReady();
    if (!audioReady_) {
        return;
    }

    if (FileExists("assets/sounds/shot.wav")) {
        shotSound_ = LoadSound("assets/sounds/shot.wav");
        externalShotReady_ = shotSound_.frameCount > 0;
    }
    if (!externalShotReady_) {
        shotSound_ = CreateTone(880.0f, 0.08f, 0.28f);
    }

    if (FileExists("assets/sounds/pickup.wav")) {
        pickupSound_ = LoadSound("assets/sounds/pickup.wav");
        externalPickupReady_ = pickupSound_.frameCount > 0;
    }
    if (!externalPickupReady_) {
        pickupSound_ = CreateTone(660.0f, 0.14f, 0.24f);
    }

    if (FileExists("assets/sounds/explosion.wav")) {
        explosionSound_ = LoadSound("assets/sounds/explosion.wav");
        externalExplosionReady_ = explosionSound_.frameCount > 0;
    }
    if (!externalExplosionReady_) {
        explosionSound_ = CreateTone(120.0f, 0.22f, 0.34f);
    }

    LoadMusicTrack(MusicTrack::Menu, "assets/music/menu_theme.wav");
    LoadMusicTrack(MusicTrack::Game, "assets/music/game_theme.wav");
    LoadMusicTrack(MusicTrack::Boss, "assets/music/boss_theme.wav");
    LoadMusicTrack(MusicTrack::GameOver, "assets/music/gameover_theme.wav");

    SetAudioStreamBufferSizeDefault(512);
    musicStream_ = LoadAudioStream(22050, 16, 1);
    musicBuffer_.reserve(512);
}

void Game::ShutdownAudio() {
    if (!audioReady_) {
        return;
    }

    UnloadSound(shotSound_);
    UnloadSound(pickupSound_);
    UnloadSound(explosionSound_);
    for (std::size_t i = 0; i < musicTracks_.size(); ++i) {
        if (externalMusicReady_[i]) {
            UnloadMusicStream(musicTracks_[i]);
        }
    }
    UnloadAudioStream(musicStream_);
    CloseAudioDevice();
    audioReady_ = false;
}

void Game::LoadMusicTrack(MusicTrack track, const char* path) {
    const std::size_t index = static_cast<std::size_t>(track);
    if (!FileExists(path)) {
        return;
    }

    musicTracks_[index] = LoadMusicStream(path);
    externalMusicReady_[index] = musicTracks_[index].stream.buffer != nullptr;
}

MusicTrack Game::TargetMusicTrack() const {
    if (state_ == GameState::GameOver || state_ == GameState::NameEntry) {
        return MusicTrack::GameOver;
    }
    if (state_ == GameState::Playing && HasLivingBoss()) {
        return MusicTrack::Boss;
    }
    if (state_ == GameState::Playing || state_ == GameState::Paused) {
        return MusicTrack::Game;
    }
    return MusicTrack::Menu;
}

void Game::PlayShotSound() {
    if (audioReady_ && saveData_.soundEnabled) {
        PlaySound(shotSound_);
    }
}

void Game::PlayPickupSound() {
    if (audioReady_ && saveData_.soundEnabled) {
        PlaySound(pickupSound_);
    }
}

void Game::PlayExplosionSound() {
    if (audioReady_ && saveData_.soundEnabled) {
        PlaySound(explosionSound_);
    }
}
#endif
