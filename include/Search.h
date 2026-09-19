#pragma once

#include "Board.h"
#include "Game.h"
#include "Piece.h"
#include "NNUE.h"
#include <vector>
#include <utility>
#include <unordered_map>
#include <string>
#include <cstdint>

class Search {
private:
    static constexpr int CHECKMATE_SCORE = 100'000;
    static constexpr int DRAW_SCORE = 0;

    NNUE nnue;

    std::unordered_map<std::string, Move> previous_best_moves;

    float maximise(Game& game, int depth, float alpha, float beta);
    float minimise(Game& game, int depth, float alpha, float beta);

    void getFeatureUpdates(std::vector<int>& after_move_features, std::vector<int>& before_move_features, int king_square, const Move& move, const Board& board);
    void makeMove(const Move& move, Game& game);
    void undoMove(const Move& move, Game& game);
    float evaluate(const Board& board);

    int getFeature(int square, const Piece& piece, int king_square) const;
    std::vector<int> getActiveFeatures(const Board& board, Colour colour) const;

    int scoreMove(const Move& move, const Board& board) const;
    void orderMoves(std::vector<Move>& moves, const Game& game);

public:
    Search();

    std::pair<Move, float> minimax(Game& game, int depth = 6);
    std::vector<std::pair<Move, float>> getScoredMoves(Game& game, int depth);
};