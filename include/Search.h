#pragma once

#include "Board.h"
#include "Game.h"
#include "Piece.h"

class Search {
private:
    static constexpr int CHECKMATE_SCORE = 100000;
    static constexpr int DRAW_SCORE = 0;

    int max_depth = 6;

    int maximise(Game& game, int depth, int alpha, int beta);
    int minimise(Game& game, int depth, int alpha, int beta);

    int pieceValue(PieceType piece_type) const;
    int evaluate(const Board& board) const;
public:
    Move minimax(Game& game);
};