#pragma once

#include "Board.h"
#include "Game.h"
#include "Piece.h"
#include <vector>
#include <unordered_map>
#include <string>

class Search {
private:
    static constexpr int CHECKMATE_SCORE = 100000;
    static constexpr int DRAW_SCORE = 0;

    // int max_depth = 10;

    std::unordered_map<std::string, Move> previous_best_moves;

    int maximise(Game& game, int depth, int alpha, int beta);
    int minimise(Game& game, int depth, int alpha, int beta);

    int scoreMove(const Move& move, const Board& board) const;
    void orderMoves(std::vector<Move>& moves, const Board& board);
    int pieceValue(PieceType piece_type) const;
    int evaluate(const Board& board) const;
public:
    Move minimax(Game& game);
};