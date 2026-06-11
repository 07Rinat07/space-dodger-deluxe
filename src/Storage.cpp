#include "Storage.hpp"
#include "Config.hpp"
#include <algorithm>
#include <cctype>
#include <functional>
#include <fstream>
#include <sstream>
#include <vector>

namespace {
int ClampHighScore(int score) {
    return std::max(0, score);
}

std::string ReadTextFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file) {
        return {};
    }

    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

int ExtractInt(const std::string& json, const std::string& key, int fallback) {
    const std::string marker = "\"" + key + "\"";
    const std::size_t keyPosition = json.find(marker);
    if (keyPosition == std::string::npos) {
        return fallback;
    }

    const std::size_t colon = json.find(':', keyPosition + marker.size());
    if (colon == std::string::npos) {
        return fallback;
    }

    std::size_t start = json.find_first_of("-0123456789", colon + 1);
    if (start == std::string::npos) {
        return fallback;
    }

    try {
        return std::stoi(json.substr(start));
    } catch (...) {
        return fallback;
    }
}

bool ExtractBool(const std::string& json, const std::string& key, bool fallback) {
    const std::string marker = "\"" + key + "\"";
    const std::size_t keyPosition = json.find(marker);
    if (keyPosition == std::string::npos) {
        return fallback;
    }

    const std::size_t colon = json.find(':', keyPosition + marker.size());
    if (colon == std::string::npos) {
        return fallback;
    }

    const std::size_t value = json.find_first_not_of(" \t\r\n", colon + 1);
    if (value == std::string::npos) {
        return fallback;
    }

    if (json.compare(value, 4, "true") == 0) {
        return true;
    }
    if (json.compare(value, 5, "false") == 0) {
        return false;
    }
    return fallback;
}

std::string ExtractString(const std::string& json, const std::string& key, const std::string& fallback) {
    const std::string marker = "\"" + key + "\"";
    const std::size_t keyPosition = json.find(marker);
    if (keyPosition == std::string::npos) {
        return fallback;
    }

    const std::size_t colon = json.find(':', keyPosition + marker.size());
    if (colon == std::string::npos) {
        return fallback;
    }

    const std::size_t quoteStart = json.find('"', colon + 1);
    if (quoteStart == std::string::npos) {
        return fallback;
    }

    const std::size_t quoteEnd = json.find('"', quoteStart + 1);
    if (quoteEnd == std::string::npos) {
        return fallback;
    }

    return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
}

std::string EscapeJsonString(const std::string& value) {
    std::string escaped;
    for (char character : value) {
        if (character == '"' || character == '\\') {
            escaped.push_back('\\');
        }
        escaped.push_back(character);
    }
    return escaped;
}

std::vector<ScoreEntry> ExtractLeaderboard(const std::string& json) {
    const std::string marker = "\"leaderboard\"";
    const std::size_t keyPosition = json.find(marker);
    if (keyPosition == std::string::npos) {
        return {};
    }

    const std::size_t openBracket = json.find('[', keyPosition + marker.size());
    const std::size_t closeBracket = json.find(']', openBracket);
    if (openBracket == std::string::npos || closeBracket == std::string::npos) {
        return {};
    }

    std::vector<ScoreEntry> scores;
    std::size_t cursor = openBracket + 1;
    while (cursor < closeBracket) {
        const std::size_t objectStart = json.find('{', cursor);
        const std::size_t numberStart = json.find_first_of("-0123456789", cursor);

        if (objectStart != std::string::npos && objectStart < closeBracket && (numberStart == std::string::npos || objectStart < numberStart)) {
            const std::size_t objectEnd = json.find('}', objectStart);
            if (objectEnd == std::string::npos || objectEnd > closeBracket) {
                break;
            }

            const std::string object = json.substr(objectStart, objectEnd - objectStart + 1);
            const std::string name = ExtractString(object, "name", "PLAYER");
            const int score = ClampHighScore(ExtractInt(object, "score", 0));
            scores.push_back({SanitizePlayerName(name), score});
            cursor = objectEnd + 1;
            continue;
        }

        if (numberStart == std::string::npos || numberStart >= closeBracket) {
            break;
        }

        try {
            scores.push_back({"PLAYER", ClampHighScore(std::stoi(json.substr(numberStart)))});
        } catch (...) {
            break;
        }

        cursor = json.find_first_of(",", numberStart);
        if (cursor == std::string::npos || cursor >= closeBracket) {
            break;
        }
        ++cursor;
    }

    return AddScoreToLeaderboard(scores, "PLAYER", 0);
}
}

std::string DifficultyToString(DifficultyLevel difficulty) {
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

DifficultyLevel DifficultyFromString(const std::string& value) {
    if (value == "easy") {
        return DifficultyLevel::Easy;
    }
    if (value == "hard") {
        return DifficultyLevel::Hard;
    }
    return DifficultyLevel::Normal;
}

DifficultyLevel NextDifficulty(DifficultyLevel difficulty) {
    switch (difficulty) {
        case DifficultyLevel::Easy:
            return DifficultyLevel::Normal;
        case DifficultyLevel::Normal:
            return DifficultyLevel::Hard;
        case DifficultyLevel::Hard:
        default:
            return DifficultyLevel::Easy;
    }
}

float DifficultySpawnMultiplier(DifficultyLevel difficulty) {
    switch (difficulty) {
        case DifficultyLevel::Easy:
            return 1.18f;
        case DifficultyLevel::Hard:
            return 0.72f;
        case DifficultyLevel::Normal:
        default:
            return 1.0f;
    }
}

float DifficultySpeedMultiplier(DifficultyLevel difficulty) {
    switch (difficulty) {
        case DifficultyLevel::Easy:
            return 0.86f;
        case DifficultyLevel::Hard:
            return 1.22f;
        case DifficultyLevel::Normal:
        default:
            return 1.0f;
    }
}

int DifficultyScoreMultiplier(DifficultyLevel difficulty) {
    switch (difficulty) {
        case DifficultyLevel::Easy:
            return 8;
        case DifficultyLevel::Hard:
            return 14;
        case DifficultyLevel::Normal:
        default:
            return 10;
    }
}

std::string SanitizePlayerName(const std::string& name) {
    std::string cleaned;
    for (char character : name) {
        const unsigned char value = static_cast<unsigned char>(character);
        if (std::isalnum(value) || character == '_' || character == '-') {
            cleaned.push_back(static_cast<char>(std::toupper(value)));
        }
        if (cleaned.size() >= 12) {
            break;
        }
    }

    return cleaned.empty() ? "PLAYER" : cleaned;
}

std::vector<ScoreEntry> AddScoreToLeaderboard(std::vector<ScoreEntry> leaderboard, const std::string& name, int score, std::size_t limit) {
    if (score > 0) {
        leaderboard.push_back({SanitizePlayerName(name), score});
    }

    std::sort(leaderboard.begin(), leaderboard.end(), [](const ScoreEntry& left, const ScoreEntry& right) {
        return left.score > right.score;
    });
    leaderboard.erase(
        std::remove_if(leaderboard.begin(), leaderboard.end(), [](const ScoreEntry& value) {
            return value.score <= 0;
        }),
        leaderboard.end()
    );

    if (leaderboard.size() > limit) {
        leaderboard.resize(limit);
    }
    return leaderboard;
}

SaveData Storage::Load(const std::string& fileName) {
    SaveData data{};
    const std::string content = ReadTextFile(fileName);
    if (content.empty()) {
        return data;
    }

    if (content.find('{') == std::string::npos) {
        try {
            data.highScore = ClampHighScore(std::stoi(content));
            data.leaderboard = AddScoreToLeaderboard({}, "PLAYER", data.highScore);
        } catch (...) {
            data.highScore = 0;
        }
        return data;
    }

    data.highScore = ClampHighScore(ExtractInt(content, "highScore", 0));
    data.difficulty = DifficultyFromString(ExtractString(content, "difficulty", "normal"));
    data.soundEnabled = ExtractBool(content, "soundEnabled", true);
    data.musicEnabled = ExtractBool(content, "musicEnabled", true);
    data.leaderboard = ExtractLeaderboard(content);
    if (data.leaderboard.empty() && data.highScore > 0) {
        data.leaderboard = AddScoreToLeaderboard({}, "PLAYER", data.highScore);
    }
    return data;
}

void Storage::Save(const SaveData& data, const std::string& fileName) {
    std::ofstream file(fileName, std::ios::trunc);
    if (!file) {
        return;
    }

    file << "{\n";
    file << "  \"highScore\": " << ClampHighScore(data.highScore) << ",\n";
    file << "  \"difficulty\": \"" << DifficultyToString(data.difficulty) << "\",\n";
    file << "  \"soundEnabled\": " << (data.soundEnabled ? "true" : "false") << ",\n";
    file << "  \"musicEnabled\": " << (data.musicEnabled ? "true" : "false") << ",\n";
    file << "  \"leaderboard\": [";
    const std::vector<ScoreEntry> leaderboard = AddScoreToLeaderboard(data.leaderboard, "PLAYER", 0);
    for (std::size_t i = 0; i < leaderboard.size(); ++i) {
        if (i > 0) {
            file << ", ";
        }
        file << "{\"name\": \"" << EscapeJsonString(leaderboard[i].name) << "\", \"score\": " << leaderboard[i].score << "}";
    }
    file << "]\n";
    file << "}\n";
}

int Storage::LoadHighScore(const std::string& fileName) {
    return Load(fileName).highScore;
}

void Storage::SaveHighScore(int score, const std::string& fileName) {
    SaveData data = Load(fileName);
    data.highScore = ClampHighScore(score);
    data.leaderboard = AddScoreToLeaderboard(data.leaderboard, "PLAYER", score);
    Save(data, fileName);
}
