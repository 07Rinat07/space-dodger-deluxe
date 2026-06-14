#include "../include/Asteroid.hpp"
#include "../include/Bullet.hpp"
#include "../include/EnemyProjectile.hpp"
#include "../include/Particle.hpp"
#include "../include/Player.hpp"
#include "../include/Pickup.hpp"
#include "../include/Storage.hpp"
#include "../include/Utils.hpp"
#include "../include/WaveSystem.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static int testsPassed = 0;
static int testsFailed = 0;

#define ASSERT_TRUE(expression) \
    do { \
        if (expression) { \
            ++testsPassed; \
        } else { \
            ++testsFailed; \
            std::cerr << "Assertion failed: " #expression << " at " << __FILE__ << ":" << __LINE__ << '\n'; \
        } \
    } while (0)

#define ASSERT_EQ(actual, expected) \
    do { \
        if ((actual) == (expected)) { \
            ++testsPassed; \
        } else { \
            ++testsFailed; \
            std::cerr << "Assertion failed: " #actual " == " #expected << " (" << (actual) << " != " << (expected) << ") at " << __FILE__ << ":" << __LINE__ << '\n'; \
        } \
    } while (0)

#define ASSERT_NEAR(actual, expected, tolerance) \
    do { \
        if (std::abs((actual) - (expected)) <= (tolerance)) { \
            ++testsPassed; \
        } else { \
            ++testsFailed; \
            std::cerr << "Assertion failed: " #actual " ~= " #expected " (" << (actual) << " != " << (expected) << ") at " << __FILE__ << ":" << __LINE__ << '\n'; \
        } \
    } while (0)

void TestClampFloat() {
    ASSERT_EQ(ClampFloat(5.0f, 0.0f, 4.0f), 4.0f);
    ASSERT_EQ(ClampFloat(-1.0f, 0.0f, 3.0f), 0.0f);
    ASSERT_EQ(ClampFloat(1.5f, 0.0f, 3.0f), 1.5f);
}

void TestDistance() {
    ASSERT_EQ(Distance({0.0f, 0.0f}, {3.0f, 4.0f}), 5.0f);
    ASSERT_EQ(Distance({1.0f, 1.0f}, {1.0f, 1.0f}), 0.0f);
}

void TestCirclesCollide() {
    ASSERT_TRUE(CirclesCollide({0.0f, 0.0f}, 1.0f, {1.5f, 0.0f}, 1.0f));
    ASSERT_TRUE(!CirclesCollide({0.0f, 0.0f}, 1.0f, {3.0f, 0.0f}, 1.0f));
}

void TestStorageSaveAndLoad() {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "space_dodger_test_high_score.txt";

    if (std::filesystem::exists(tempPath)) {
        std::filesystem::remove(tempPath);
    }

    const int expectedScore = 12345;
    Storage::SaveHighScore(expectedScore, tempPath.string());
    ASSERT_EQ(Storage::LoadHighScore(tempPath.string()), expectedScore);

    std::filesystem::remove(tempPath);
    ASSERT_TRUE(!std::filesystem::exists(tempPath));
}

void TestStorageJsonSettingsRoundTrip() {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "space_dodger_settings_save.json";
    SaveData expected{};
    expected.highScore = 777;
    expected.difficulty = DifficultyLevel::Hard;
    expected.soundEnabled = false;
    expected.musicEnabled = true;
    expected.leaderboard = {{"ACE", 777}, {"BOB", 450}, {"CAT", 120}};

    Storage::Save(expected, tempPath.string());
    const SaveData actual = Storage::Load(tempPath.string());

    ASSERT_EQ(actual.highScore, 777);
    ASSERT_TRUE(actual.difficulty == DifficultyLevel::Hard);
    ASSERT_TRUE(!actual.soundEnabled);
    ASSERT_TRUE(actual.musicEnabled);
    ASSERT_EQ(actual.leaderboard.size(), static_cast<std::size_t>(3));
    ASSERT_EQ(actual.leaderboard[0].name, std::string("ACE"));
    ASSERT_EQ(actual.leaderboard[0].score, 777);
    ASSERT_EQ(actual.leaderboard[1].name, std::string("BOB"));
    ASSERT_EQ(actual.leaderboard[1].score, 450);
    ASSERT_EQ(actual.leaderboard[2].name, std::string("CAT"));
    ASSERT_EQ(actual.leaderboard[2].score, 120);

    std::filesystem::remove(tempPath);
}

void TestStorageIgnoresInvalidScores() {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "space_dodger_invalid_high_score.txt";

    {
        std::ofstream file(tempPath);
        file << -50;
    }
    ASSERT_EQ(Storage::LoadHighScore(tempPath.string()), 0);

    {
        std::ofstream file(tempPath);
        file << "broken";
    }
    ASSERT_EQ(Storage::LoadHighScore(tempPath.string()), 0);

    Storage::SaveHighScore(-25, tempPath.string());
    ASSERT_EQ(Storage::LoadHighScore(tempPath.string()), 0);

    std::filesystem::remove(tempPath);
}

void TestStorageMissingFileReturnsZero() {
    const std::filesystem::path tempPath = std::filesystem::temp_directory_path() / "space_dodger_missing_high_score.txt";

    if (std::filesystem::exists(tempPath)) {
        std::filesystem::remove(tempPath);
    }

    ASSERT_EQ(Storage::LoadHighScore(tempPath.string()), 0);
}

void TestDifficultyHelpers() {
    ASSERT_EQ(DifficultyToString(DifficultyLevel::Easy), std::string("easy"));
    ASSERT_EQ(DifficultyToString(DifficultyLevel::Normal), std::string("normal"));
    ASSERT_EQ(DifficultyToString(DifficultyLevel::Hard), std::string("hard"));
    ASSERT_TRUE(DifficultyFromString("easy") == DifficultyLevel::Easy);
    ASSERT_TRUE(DifficultyFromString("hard") == DifficultyLevel::Hard);
    ASSERT_TRUE(DifficultyFromString("unknown") == DifficultyLevel::Normal);
    ASSERT_TRUE(NextDifficulty(DifficultyLevel::Easy) == DifficultyLevel::Normal);
    ASSERT_TRUE(NextDifficulty(DifficultyLevel::Normal) == DifficultyLevel::Hard);
    ASSERT_TRUE(NextDifficulty(DifficultyLevel::Hard) == DifficultyLevel::Easy);
    ASSERT_TRUE(DifficultySpawnMultiplier(DifficultyLevel::Hard) < DifficultySpawnMultiplier(DifficultyLevel::Easy));
    ASSERT_TRUE(DifficultySpeedMultiplier(DifficultyLevel::Hard) > DifficultySpeedMultiplier(DifficultyLevel::Easy));
    ASSERT_TRUE(DifficultyScoreMultiplier(DifficultyLevel::Hard) > DifficultyScoreMultiplier(DifficultyLevel::Easy));
}

void TestLeaderboardKeepsBestScores() {
    std::vector<ScoreEntry> leaderboard = {{"ONE", 100}, {"TWO", 400}, {"THREE", 50}};
    leaderboard = AddScoreToLeaderboard(leaderboard, "new player!", 250, 3);

    ASSERT_EQ(leaderboard.size(), static_cast<std::size_t>(3));
    ASSERT_EQ(leaderboard[0].score, 400);
    ASSERT_EQ(leaderboard[1].name, std::string("NEWPLAYER"));
    ASSERT_EQ(leaderboard[1].score, 250);
    ASSERT_EQ(leaderboard[2].score, 100);

    leaderboard = AddScoreToLeaderboard(leaderboard, "bad", -20, 3);
    ASSERT_EQ(leaderboard.size(), static_cast<std::size_t>(3));
    ASSERT_EQ(SanitizePlayerName(""), std::string("PLAYER"));
    ASSERT_EQ(SanitizePlayerName("a_b-12!"), std::string("A_B-12"));
}

void TestWaveSystem() {
    const WaveInfo firstWave = ComputeWaveInfo(0.0f);
    const WaveInfo thirdWave = ComputeWaveInfo(60.0f);
    const WaveInfo lateWave = ComputeWaveInfo(180.0f);

    ASSERT_EQ(firstWave.number, 1);
    ASSERT_TRUE(!firstWave.bossWave);
    ASSERT_EQ(thirdWave.number, 3);
    ASSERT_TRUE(thirdWave.bossWave);
    ASSERT_TRUE(lateWave.spawnDelayMultiplier < firstWave.spawnDelayMultiplier);
    ASSERT_TRUE(lateWave.enemySpeedMultiplier > firstWave.enemySpeedMultiplier);
}

void TestAsteroidMovementAndBounds() {
    Asteroid asteroid({10.0f, 20.0f}, {30.0f, 40.0f}, 12.0f, 1.5f);

    asteroid.Update(0.5f);

    ASSERT_NEAR(asteroid.GetPosition().x, 25.0f, 0.001f);
    ASSERT_NEAR(asteroid.GetPosition().y, 40.0f, 0.001f);
    ASSERT_EQ(asteroid.GetRadius(), 12.0f);
    ASSERT_TRUE(asteroid.GetType() == AsteroidType::Rock);
    ASSERT_EQ(asteroid.GetHealth(), 1);
    ASSERT_TRUE(!asteroid.IsOffScreen());

    Asteroid bottomOffscreen({100.0f, 800.0f}, {0.0f, 0.0f}, 20.0f, 0.0f);
    Asteroid leftOffscreen({-150.0f, 100.0f}, {0.0f, 0.0f}, 20.0f, 0.0f);
    Asteroid rightOffscreen({1150.0f, 100.0f}, {0.0f, 0.0f}, 20.0f, 0.0f);

    ASSERT_TRUE(bottomOffscreen.IsOffScreen());
    ASSERT_TRUE(leftOffscreen.IsOffScreen());
    ASSERT_TRUE(rightOffscreen.IsOffScreen());
}

void TestAsteroidTypesAndDamage() {
    Asteroid fast({0.0f, 0.0f}, {0.0f, 0.0f}, 16.0f, 0.0f, AsteroidType::Fast);
    Asteroid heavy({0.0f, 0.0f}, {0.0f, 0.0f}, 32.0f, 0.0f, AsteroidType::Heavy);
    Asteroid boss({0.0f, 0.0f}, {0.0f, 0.0f}, 74.0f, 0.0f, AsteroidType::BossCruiser);
    Asteroid carrier({0.0f, 0.0f}, {0.0f, 0.0f}, 86.0f, 0.0f, AsteroidType::BossCarrier);

    ASSERT_TRUE(fast.GetType() == AsteroidType::Fast);
    ASSERT_TRUE(!fast.HasNearMissAwarded());
    fast.MarkNearMissAwarded();
    ASSERT_TRUE(fast.HasNearMissAwarded());
    ASSERT_EQ(fast.GetHealth(), 1);
    ASSERT_TRUE(fast.GetScoreValue() > 0);
    ASSERT_TRUE(fast.TakeDamage(1));

    ASSERT_TRUE(heavy.GetType() == AsteroidType::Heavy);
    ASSERT_EQ(heavy.GetHealth(), 2);
    ASSERT_TRUE(!heavy.TakeDamage(1));
    ASSERT_EQ(heavy.GetHealth(), 1);
    ASSERT_TRUE(heavy.TakeDamage(1));

    ASSERT_TRUE(boss.GetType() == AsteroidType::BossCruiser);
    ASSERT_TRUE(boss.GetHealth() > heavy.GetHealth());
    ASSERT_EQ(boss.GetMaxHealth(), boss.GetHealth());
    ASSERT_TRUE(boss.GetScoreValue() > heavy.GetScoreValue());
    ASSERT_TRUE(carrier.GetHealth() > boss.GetHealth());
}

void TestEnemyProjectileMovementAndBounds() {
    EnemyProjectile projectile({20.0f, 30.0f}, {40.0f, 120.0f}, 9.0f);

    projectile.Update(0.5f);

    ASSERT_NEAR(projectile.GetPosition().x, 40.0f, 0.001f);
    ASSERT_NEAR(projectile.GetPosition().y, 90.0f, 0.001f);
    ASSERT_NEAR(projectile.GetRadius(), 9.0f, 0.001f);
    ASSERT_TRUE(!projectile.HasNearMissAwarded());
    projectile.MarkNearMissAwarded();
    ASSERT_TRUE(projectile.HasNearMissAwarded());
    ASSERT_TRUE(!projectile.IsOffScreen());

    projectile.Update(6.0f);
    ASSERT_TRUE(projectile.IsOffScreen());
}

void TestEnemyProjectilePatterns() {
    EnemyProjectile sine({100.0f, 20.0f}, {0.0f, 100.0f}, 8.0f, ProjectilePattern::Sine, 30.0f, 3.0f);
    sine.Update(0.5f);
    ASSERT_TRUE(sine.GetPattern() == ProjectilePattern::Sine);
    ASSERT_TRUE(sine.GetPosition().x > 120.0f);
    ASSERT_NEAR(sine.GetPosition().y, 70.0f, 0.001f);

    EnemyProjectile arc({10.0f, 10.0f}, {20.0f, 100.0f}, 7.0f, ProjectilePattern::Arc, 200.0f);
    arc.Update(0.5f);
    ASSERT_TRUE(arc.GetPattern() == ProjectilePattern::Arc);
    ASSERT_NEAR(arc.GetPosition().x, 20.0f, 0.001f);
    ASSERT_TRUE(arc.GetPosition().y > 60.0f);

    EnemyProjectile drift({50.0f, 20.0f}, {0.0f, 100.0f}, 7.0f, ProjectilePattern::Drift, 120.0f, 5.0f);
    drift.Update(0.5f);
    ASSERT_TRUE(drift.GetPattern() == ProjectilePattern::Drift);
    ASSERT_TRUE(drift.GetPosition().x > 50.0f);
}

void TestBulletMovementAndBounds() {
    Bullet bullet({40.0f, 100.0f}, 200.0f);

    bullet.Update(0.25f);

    ASSERT_NEAR(bullet.GetPosition().x, 40.0f, 0.001f);
    ASSERT_NEAR(bullet.GetPosition().y, 50.0f, 0.001f);
    ASSERT_NEAR(bullet.GetRadius(), 5.0f, 0.001f);
    ASSERT_TRUE(!bullet.IsOffScreen());

    bullet.Update(1.0f);
    ASSERT_TRUE(bullet.IsOffScreen());

    Bullet diagonal({40.0f, 100.0f}, Vector2{20.0f, -160.0f});
    diagonal.Update(0.5f);
    ASSERT_NEAR(diagonal.GetPosition().x, 50.0f, 0.001f);
    ASSERT_NEAR(diagonal.GetPosition().y, 20.0f, 0.001f);
}

void TestPickupMovementTypeAndBounds() {
    Pickup scorePickup({100.0f, 50.0f}, PickupType::Score);
    Pickup shieldPickup({120.0f, 60.0f}, PickupType::Shield);
    Pickup rapidPickup({140.0f, 60.0f}, PickupType::RapidFire);
    Pickup spreadPickup({160.0f, 60.0f}, PickupType::SpreadShot);

    scorePickup.Update(2.0f);

    ASSERT_NEAR(scorePickup.GetPosition().x, 100.0f, 0.001f);
    ASSERT_NEAR(scorePickup.GetPosition().y, 290.0f, 0.001f);
    ASSERT_EQ(scorePickup.GetRadius(), 16.0f);
    ASSERT_TRUE(scorePickup.GetType() == PickupType::Score);
    ASSERT_TRUE(shieldPickup.GetType() == PickupType::Shield);
    ASSERT_TRUE(rapidPickup.GetType() == PickupType::RapidFire);
    ASSERT_TRUE(spreadPickup.GetType() == PickupType::SpreadShot);
    ASSERT_TRUE(!scorePickup.IsOffScreen());

    Pickup offscreenPickup({100.0f, 760.0f}, PickupType::Score);
    ASSERT_TRUE(offscreenPickup.IsOffScreen());
}

void TestParticleMovementAndLifetime() {
    Particle particle({5.0f, 10.0f}, {20.0f, -10.0f}, 1.0f, Color{255, 255, 255, 255});

    ASSERT_TRUE(!particle.IsDead());
    particle.Update(0.25f);

    ASSERT_NEAR(particle.GetPosition().x, 10.0f, 0.001f);
    ASSERT_NEAR(particle.GetPosition().y, 7.5f, 0.001f);
    ASSERT_NEAR(particle.GetLife(), 0.75f, 0.001f);
    ASSERT_TRUE(!particle.IsDead());

    particle.Update(0.75f);
    ASSERT_TRUE(particle.IsDead());
}

void TestPlayerResetAndShieldTimer() {
    Player player;

    ASSERT_NEAR(player.GetPosition().x, 500.0f, 0.001f);
    ASSERT_NEAR(player.GetPosition().y, 605.0f, 0.001f);
    ASSERT_NEAR(player.GetRadius(), 22.0f, 0.001f);
    ASSERT_TRUE(!player.HasShield());

    player.ActivateShield(1.0f);
    ASSERT_TRUE(player.HasShield());

    player.Update(0.4f);
    ASSERT_TRUE(player.HasShield());

    player.Update(0.6f);
    ASSERT_TRUE(!player.HasShield());

    player.ActivateShield(3.0f);
    player.Reset();
    ASSERT_TRUE(!player.HasShield());
}

int main() {
    TestClampFloat();
    TestDistance();
    TestCirclesCollide();
    TestStorageSaveAndLoad();
    TestStorageJsonSettingsRoundTrip();
    TestStorageIgnoresInvalidScores();
    TestStorageMissingFileReturnsZero();
    TestDifficultyHelpers();
    TestLeaderboardKeepsBestScores();
    TestWaveSystem();
    TestAsteroidMovementAndBounds();
    TestAsteroidTypesAndDamage();
    TestEnemyProjectileMovementAndBounds();
    TestEnemyProjectilePatterns();
    TestBulletMovementAndBounds();
    TestPickupMovementTypeAndBounds();
    TestParticleMovementAndLifetime();
    TestPlayerResetAndShieldTimer();

    std::cout << "Tests passed: " << testsPassed << "\n";
    std::cout << "Tests failed: " << testsFailed << "\n";

    return testsFailed == 0 ? 0 : 1;
}
