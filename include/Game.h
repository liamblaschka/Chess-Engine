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
    Draw
};

struct GameHistory {
    std::string position_key;
    int halfmove_clock;
};

class Game {
private:
    Board board;
    MoveGenerator move_generator;
    std::unordered_map<std::string, int> positions;
    std::string current_position;
    std::vector<Move> current_legal_moves;
    int halfmove_clock = 0;
    std::vector<GameHistory> history;
public:
    Game();
    void trackPosition(const std::vector<Move>& legal_moves);
    GameState getGameState(const std::vector<Move>& legal_moves) const;
    std::vector<Move> getLegalMoves();
    void makeMove(const Move& move);
    void undoMove();
    Colour getTurn() const;
    Board& getBoard();
    const Board& getBoard() const;

    void setPosition(const std::string& fen);
};
