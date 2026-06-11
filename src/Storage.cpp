#include "Storage.hpp"
#include "Config.hpp"
#include <fstream>

int Storage::LoadHighScore() {
    std::ifstream file(cfg::SaveFileName);
    int value = 0;

    if (file >> value) {
        return value;
    }

    return 0;
}

void Storage::SaveHighScore(int score) {
    std::ofstream file(cfg::SaveFileName, std::ios::trunc);
    if (file) {
        file << score;
    }
}
