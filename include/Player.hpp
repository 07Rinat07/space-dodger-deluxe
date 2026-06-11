#pragma once

#include "raylib.h"

class Player {
public:
    Player();

    void Reset();
    void Update(float dt);
    void Draw() const;

    Vector2 GetPosition() const;
    float GetRadius() const;

    void ActivateShield(float seconds);
    bool HasShield() const;

private:
    Vector2 position_{};
    float radius_{};
    float speed_{};
    float shieldTimeLeft_{};

    void KeepInsideScreen();
};
