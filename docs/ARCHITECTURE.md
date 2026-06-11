# Architecture notes

This document explains how the game is organized.

## Main idea

The game uses a classic real-time game loop:

```text
while window is open:
    read input
    update world
    draw world
```

The loop is implemented in `Game::Run()`.

## Main classes

### Game

`Game` is the central controller.

Responsibilities:

- owns the window lifecycle;
- stores the current game state;
- updates all game objects;
- draws all game objects;
- creates asteroids and pickups;
- checks collisions;
- saves high score.

### Player

`Player` stores and updates the spaceship.

Responsibilities:

- movement by keyboard;
- screen boundary limits;
- shield timer;
- drawing the ship.

### Asteroid

`Asteroid` represents a dangerous falling object.

Responsibilities:

- movement;
- procedural asteroid drawing;
- off-screen detection.

### Pickup

`Pickup` represents a bonus object.

Types:

- `Score` gives bonus points;
- `Shield` gives temporary protection.

### Particle

`Particle` is used for simple visual effects after pickups, shield collision and game over.

### Starfield

`Starfield` creates a moving background and makes the game look more alive.

### Storage

`Storage` reads and writes the high score to a text file.

## Game states

The enum `GameState` has four values:

```cpp
enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver
};
```

This is better than many boolean variables like `isPaused`, `isGameOver`, `isInMenu`, because only one state can be active at a time.

## Collision detection

The project uses circle collision:

```cpp
bool CirclesCollide(Vector2 a, float radiusA, Vector2 b, float radiusB)
```

Two objects collide when the distance between their centers is less than or equal to the sum of their radiuses.

This is simple, fast and good enough for an arcade game.

## Delta time

The game uses `GetFrameTime()` from raylib.

This returns time in seconds since the previous frame.

Movement uses this formula:

```cpp
position += velocity * deltaTime;
```

Because of this, the game speed does not depend directly on FPS.
