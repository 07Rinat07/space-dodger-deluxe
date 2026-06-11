#include "WaveSystem.hpp"
#include <algorithm>

WaveInfo ComputeWaveInfo(float survivedTime) {
    const int wave = std::max(1, 1 + static_cast<int>(survivedTime / 30.0f));
    WaveInfo info{};
    info.number = wave;
    info.bossWave = wave % 3 == 0;
    info.spawnDelayMultiplier = std::max(0.55f, 1.0f - static_cast<float>(wave - 1) * 0.06f);
    info.enemySpeedMultiplier = 1.0f + static_cast<float>(wave - 1) * 0.08f;
    return info;
}
