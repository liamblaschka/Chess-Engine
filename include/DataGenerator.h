#pragma once

#include "Search.h"
#include "Board.h"
#include "Game.h"
#include <random>
#include <string>

class DataGenerator {
private:
    // static constexpr int PLAY_DEPTH = 3;
    // static constexpr int EVAL_DEPTH = 4;

    static constexpr int PLAY_DEPTH = 3;
    static constexpr int EVAL_DEPTH = 5;
    static constexpr int NUM_GAMES = 1'000;
    static constexpr double SAMPLE_PROBABILITY = 0.1;
    static constexpr int RANDOM_PLIES = 8;
    static constexpr float RANDOM_MOVE_THRESHOLD = 30.0f;
    static constexpr float CHECKMATE_SCORE = 100'000.0f;

    Game game;
    Search search;

    std::random_device rd;
    std::mt19937 rng;
    std::uniform_real_distribution<double> prob_distribution;

public:
    DataGenerator();

    void playGames();

    std::string makeTimestampedFilename();
};