#pragma once

#include "raylib.h"

// Bonus item type.
// Score adds points, Shield protects the player from one collision window.
enum class PickupType {
    Score,
    Shield
};

class Pickup {
public:
    Pickup(Vector2 position, PickupType type);

    void Update(float dt);
    void Draw() const;

    Vector2 GetPosition() const;
    float GetRadius() const;
    PickupType GetType() const;
    bool IsOffScreen() const;

private:
    Vector2 position_{};
    PickupType type_{};
    float radius_{};
    float speed_{};
    float pulse_{};
};
