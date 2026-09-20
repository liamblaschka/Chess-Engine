#include "DataGenerator.h"
#include "Search.h"
#include "Board.h"
#include "Game.h"
#include "Move.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <ctime>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <iostream>

DataGenerator::DataGenerator() : rng(rd()), prob_distribution(0.0, 1.0), search(game) {}

void DataGenerator::playGames() {
    std::filesystem::create_directories("data");
    std::ofstream file("data/" + makeTimestampedFilename());
    if (!file) {
        throw std::runtime_error("Failed to open file.");
    }
    file << "FEN,Evaluation,GameResult" << "\n";
    
    auto progress_start = std::chrono::steady_clock::now();

    std::size_t num_sampled_positions = 0;
    for (int i = 0; i < NUM_GAMES; i++) {
        game = Game();
        search.reset();

        int ply_count = 0;
        std::vector<std::string> saved_positions;
        GameState game_state = GameState::Playing;
        while (game_state == GameState::Playing || game_state == GameState::Check) {
            if (ply_count < RANDOM_PLIES) {
                std::vector<std::pair<Move, float>> scored_moves = search.getScoredMoves(PLAY_DEPTH);
                float best_score = scored_moves[0].second;
                if (game.getTurn() == Colour::White) {
                    for (const auto& [move, score] : scored_moves) {
                        best_score = std::max(best_score, score);
                    }
                } else {
                    for (const auto& [move, score] : scored_moves) {
                        best_score = std::min(best_score, score);
                    }
                }
                
                std::vector<std::pair<Move, float>> candidate_moves;
                for (const auto& [move, score] : scored_moves) {
                    if (std::abs(best_score - score) <= RANDOM_MOVE_THRESHOLD) {
                        candidate_moves.push_back({move, score});
                    }
                }

                std::uniform_int_distribution<std::size_t> dist(0, candidate_moves.size() - 1);
                Move selected_move = candidate_moves[dist(rng)].first;
                
                search.makeMove(selected_move);
                ply_count++;
            } else if (prob_distribution(rng) < SAMPLE_PROBABILITY) {
                auto [best_move, best_score] = search.run(EVAL_DEPTH);

                if (std::abs(best_score) < CHECKMATE_SCORE) {
                    std::stringstream position_entry;
                    position_entry << game.getPositionFen();
                    position_entry << ',' << std::fixed << std::setprecision(2) << best_score;
                    saved_positions.push_back(position_entry.str());

                    num_sampled_positions++;
                }
                
                search.makeMove(best_move);
                ply_count++;
            } else {
                auto [best_move, best_score] = search.run(PLAY_DEPTH);
                search.makeMove(best_move);
                ply_count++;
            }

            game_state = game.getGameState();
        }

        std::string game_result;
        if (game_state == GameState::Checkmate) {
            if (game.getTurn() == Colour::Black) {
                game_result = ",1";
            } else {
                game_result = ",0";
            }
        } else {
            game_result = ",0.5";
        }
        for (const std::string& position : saved_positions) {
            file << position << game_result << "\n";
        }


        if ((i + 1) % 100 == 0) {
            auto now = std::chrono::steady_clock::now();
            double seconds = std::chrono::duration<double>(now - progress_start).count();

            std::cout << "Games: " << i + 1 << " / " << NUM_GAMES
                      << " | Positions: " << num_sampled_positions
                      << " | Time: " << seconds << "s\n";
            
            progress_start = now;
        }
    }
}

std::string DataGenerator::makeTimestampedFilename() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&time);
    std::stringstream ss;
    ss << "selfplay_" << std::put_time(&local_time, "%d-%m-%Y_%H-%M") << ".csv";

    return ss.str();
}