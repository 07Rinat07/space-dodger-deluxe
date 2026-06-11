#include "Game.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        Game game;
        game.Run();
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return 1;
    }
}
