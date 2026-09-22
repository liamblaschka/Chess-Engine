#pragma once

#include "Board.h"
#include "Game.h"
#include "Piece.h"
#include "NNUE.h"
#include "TranspositionTable.h"
#include <vector>
#include <utility>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <random>

class Search {
private:
    static constexpr int CHECKMATE_SCORE = 100'000;
    static constexpr int DRAW_SCORE = 0;

    NNUE nnue;
    Game& game;

    // std::unordered_map<std::string, Move> previous_best_moves;

    TranspositionTable transposition_table;

    std::atomic<bool> running;

    std::atomic<bool> result_ready;

    std::random_device rd;

    std::pair<Move, float> result;

    std::mutex worker_mutex;

    std::condition_variable work_available;
    std::condition_variable work_finished;
    int workers_to_start;
    int workers_finished;
    std::vector<std::thread> workers;

    int root_depth;


    float negamax(int depth, float alpha, float beta, Game& game, NNUE& nnue, std::mt19937& rng);
    std::pair<Move, float> negamaxRoot(int depth, Game& game, NNUE& nnue, std::mt19937& rng);

    void workerLoop();

    void getFeatureUpdates(std::vector<int>& after_move_features, std::vector<int>& before_move_features, int king_square, const Move& move, const Board& board);
    void makeMove(const Move& move, Game& game, NNUE& nnue);
    void undoMove(const Move& move, Game& game, NNUE& nnue);
    float evaluate(Colour side_to_move, NNUE& nnue) const;

    int getFeature(int square, const Piece& piece, int king_square) const;
    std::vector<int> getActiveFeatures(const Board& board, Colour colour) const;

    int scoreMove(const Move& move, const Board& board) const;
    void orderMoves(std::vector<Move>& moves, Game& game, std::mt19937& rng);

public:
    Search(Game& game);

    std::pair<Move, float> run(int depth = 6);

    // std::vector<std::pair<Move, float>> getScoredMoves(int depth);

    // void makeBaseMove(const Move& move);
    // void undoBaseMove(const Move& move);

    void makeMove(const Move& move);
    void undoMove(const Move& move);

    void reset();

    ~Search();
};