#pragma once

#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"
#include <vector>

enum class GameState {
    Playing,
    Check,
    Checkmate,
    Stalemate,
    Draw
};

class Game {
private:
    Board board;
    MoveGenerator move_generator;
    GameState getGameState(const std::vector<Move>& legal_moves) const;
public:

};
