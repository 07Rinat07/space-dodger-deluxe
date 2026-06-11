#pragma once

struct WaveInfo {
    int number = 1;
    bool bossWave = false;
    float spawnDelayMultiplier = 1.0f;
    float enemySpeedMultiplier = 1.0f;
};

WaveInfo ComputeWaveInfo(float survivedTime);
