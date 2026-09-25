#pragma once

#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>

enum class GameState {
    Playing,
    Check,
    Checkmate,
    Draw
};

struct GameHistory {
    std::uint64_t position_key;
    int halfmove_clock;
    int fullmove_number;
};

class Game {
private:
    Board board;
    MoveGenerator move_generator;
    std::unordered_map<std::uint64_t, int> position_counts;
    std::uint64_t current_repetition_key;
    int halfmove_clock = 0;
    int fullmove_number = 1;
    std::vector<GameHistory> history;
public:
    Game();
    void trackPosition();
    GameState getGameState() const;
    const std::vector<Move>& getLegalMoves() const;
    void makeMove(const Move& move);
    void undoMove();
    Colour getTurn() const;
    Board& getBoard();
    const Board& getBoard() const;

    std::string getPositionFen() const;
    void setPosition(const std::string& fen);

    std::uint64_t getZobristKey() const;
};
