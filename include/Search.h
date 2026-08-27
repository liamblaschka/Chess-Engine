#pragma once

#include "Board.h"
#include "Game.h"
#include "Piece.h"
#include "NNUE.h"
#include <vector>
#include <unordered_map>
#include <string>

class Search {
private:
    static constexpr int CHECKMATE_SCORE = 100000;
    static constexpr int DRAW_SCORE = 0;

    NNUE nnue;

    std::vector<float> white_acc_values;
    std::vector<float> black_acc_values;

    // int max_depth = 10;

    std::unordered_map<std::string, Move> previous_best_moves;

    float maximise(Game& game, int depth, float alpha, float beta);
    float minimise(Game& game, int depth, float alpha, float beta);

    void makeMove(const Move& move, Game& game);
    void undoMove(const Move& move, Game& game);
    float evaluate(const Board& board);

    int getFeature(int square, const Piece& piece) const;
    std::vector<int> getActiveFeatures(Board& board) const;

    int scoreMove(const Move& move, const Board& board) const;
    void orderMoves(std::vector<Move>& moves, const Board& board);


    // int pieceValue(PieceType piece_type) const;
public:
    Search();

    Move minimax(Game& game);
};