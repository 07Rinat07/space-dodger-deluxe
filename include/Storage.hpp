#pragma once

#include "Config.hpp"
#include <string>
#include <vector>

enum class DifficultyLevel {
    Easy,
    Normal,
    Hard
};

struct ScoreEntry {
    std::string name = "PLAYER";
    int score = 0;
};

struct SaveData {
    int highScore = 0;
    DifficultyLevel difficulty = DifficultyLevel::Normal;
    bool soundEnabled = true;
    bool musicEnabled = true;
    std::vector<ScoreEntry> leaderboard;
};

class Storage {
public:
    static SaveData Load(const std::string& fileName = cfg::SaveFileName);
    static void Save(const SaveData& data, const std::string& fileName = cfg::SaveFileName);
    static int LoadHighScore(const std::string& fileName = cfg::SaveFileName);
    static void SaveHighScore(int score, const std::string& fileName = cfg::SaveFileName);
};

std::string DifficultyToString(DifficultyLevel difficulty);
DifficultyLevel DifficultyFromString(const std::string& value);
DifficultyLevel NextDifficulty(DifficultyLevel difficulty);
float DifficultySpawnMultiplier(DifficultyLevel difficulty);
float DifficultySpeedMultiplier(DifficultyLevel difficulty);
int DifficultyScoreMultiplier(DifficultyLevel difficulty);
std::string SanitizePlayerName(const std::string& name);
std::vector<ScoreEntry> AddScoreToLeaderboard(std::vector<ScoreEntry> leaderboard, const std::string& name, int score, std::size_t limit = 5);
