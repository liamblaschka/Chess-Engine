#pragma once

#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"
#include <vector>
#include <string>
#include <unordered_map>

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
    std::unordered_map<std::string, int> positions;
public:
    void trackPosition(const std::vector<Move>& legal_moves);
    GameState getGameState(const std::vector<Move>& legal_moves) const;
};
