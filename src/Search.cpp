#include "Search.h"
#include "Game.h"
#include "Board.h"
#include "Move.h"
#include "SquareConstants.h"
#include "NNUE.h"
#include <array>
#include <vector>
#include <utility>
#include <limits>
#include <algorithm>
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// #include <iostream>

Search::Search(Game& game) : game(game), running(true), result_ready(false), workers_to_start(0), workers_finished(0) {
    NNUE::loadModel("models/nnue.bin");

    nnue.refreshWhiteAccumulator(getActiveFeatures(game.getBoard(), Colour::White));
    nnue.refreshBlackAccumulator(getActiveFeatures(game.getBoard(), Colour::Black));

    int num_workers = std::thread::hardware_concurrency();
    if (num_workers == 0) {
        num_workers = 1;
    }
    workers.reserve(num_workers);
    for (int i = 0; i < num_workers; i++) {
        workers.emplace_back(&Search::workerLoop, this);
    }
}

float Search::negamax(int depth, float alpha, float beta, Game& game, NNUE& nnue, std::mt19937& rng) {
    float alpha_original = alpha;

    std::uint64_t zobrist_key = game.getZobristKey();

    // {
    //     std::lock_guard<std::mutex> lock(tt_mutex);
    
    //     const TTEntry* tt_entry = transposition_table.probe(zobrist_key);
    //     if (tt_entry != nullptr && tt_entry->depth >= depth) {
    //         if ((tt_entry->flag == TTFlag::Exact)
    //             || (tt_entry->flag == TTFlag::LowerBound && tt_entry->value >= beta)
    //             || (tt_entry->flag == TTFlag::UpperBound && tt_entry->value <= alpha))
    //         {
    //             return tt_entry->value;
    //         }
    //     }
    // }


    TTEntry tt_entry;
    if (transposition_table.probe(zobrist_key, tt_entry) && tt_entry.depth >= depth) {
        if ((tt_entry.flag == TTFlag::Exact)
            || (tt_entry.flag == TTFlag::LowerBound && tt_entry.value >= beta)
            || (tt_entry.flag == TTFlag::UpperBound && tt_entry.value <= alpha))
        {
            return tt_entry.value;
        }
    }

    GameState game_state = game.getGameState();
    if (game_state == GameState::Draw) {
        return DRAW_SCORE;
    }
    if (game_state == GameState::Checkmate) {
        return -CHECKMATE_SCORE - depth;
    }
    if (depth == 0) {
        float value = evaluate(game.getTurn(), nnue);

        if (game.getTurn() == Colour::Black) {
            return -value;
        }
        return value;
    }

    Move best_move;
    float best_value = std::numeric_limits<float>::lowest();

    std::vector<Move> moves = game.getLegalMoves();
    orderMoves(moves, game, rng);

    best_value = std::numeric_limits<float>::lowest();
    for (const Move& move : moves) {
        if (result_ready) {
            return 0.0f;
        }

        makeMove(move, game, nnue);
        float value = -negamax(depth - 1, -beta, -alpha, game, nnue, rng);
        undoMove(move, game, nnue);

        if (value > best_value) {
            best_value = value;
            best_move = move;
        }

        alpha = std::max(alpha, value);
        if (alpha >= beta) {
            break;
        }
    }

    // {
    //     std::lock_guard<std::mutex> lock(tt_mutex);

    //     if (value <= alpha_original) {
    //         transposition_table.store(zobrist_key, TTFlag::UpperBound, value, depth);
    //     } else if (value >= beta) {
    //         transposition_table.store(zobrist_key, TTFlag::LowerBound, value, depth);
    //     } else {
    //         transposition_table.store(zobrist_key, TTFlag::Exact, value, depth);
    //     }
    // }


    if (best_value <= alpha_original) {
        transposition_table.store(zobrist_key, TTFlag::UpperBound, best_value, depth, best_move);
    } else if (best_value >= beta) {
        transposition_table.store(zobrist_key, TTFlag::LowerBound, best_value, depth, best_move);
    } else {
        transposition_table.store(zobrist_key, TTFlag::Exact, best_value, depth, best_move);
    }

    return best_value;
}

std::pair<Move, float> Search::negamaxRoot(int depth, Game& game, NNUE& nnue, std::mt19937& rng) {
    std::vector<Move> moves = game.getLegalMoves();
    orderMoves(moves, game, rng);

    float alpha = std::numeric_limits<float>::lowest();
    float beta = std::numeric_limits<float>::max();

    Move best_move;
    float best_value = std::numeric_limits<float>::lowest();

    for (const Move& move : moves) {
        if (result_ready) {
            break;
        }
        
        makeMove(move, game, nnue);
        float value = -negamax(depth - 1, -beta, -alpha, game, nnue, rng);
        undoMove(move, game, nnue);

        if (value > best_value) {
            best_value = value;
            best_move = move;
        }

        alpha = std::max(alpha, value);
    }

    return {best_move, best_value};
}

std::pair<Move, float> Search::run(int depth) {
    {
        std::lock_guard<std::mutex> lock(worker_mutex);

        root_depth = depth;

        workers_to_start = workers.size();
        workers_finished = 0;
    }

    result_ready = false;

    // std::cout << "RUN: notifying workers\n"; //test

    work_available.notify_all();

    {
        std::unique_lock<std::mutex> lock(worker_mutex);

        while (workers_finished < workers.size()) {
            work_finished.wait(lock);
        }
    }

    // std::cout << "RUN: workers finished\n"; //test

    return result;
}

void Search::workerLoop() {
    std::mt19937 rng(rd());

    while (running) {
        {
            std::unique_lock<std::mutex> lock(worker_mutex);

            while (running && workers_to_start == 0) {
                work_available.wait(lock);
            }
            if (!running) {
                return;
            }

            workers_to_start--;
        }

        // std::cout << "WORKER: starting search\n"; //test

        Game thread_game = this->game;
        NNUE thread_nnue = this->nnue;
        
        std::pair<Move, float> worker_result = negamaxRoot(root_depth, thread_game, thread_nnue, rng);

        // std::cout << "WORKER: search finished\n"; //test

        if (!result_ready) {
            {
                std::lock_guard<std::mutex> lock(worker_mutex);
                result = worker_result;
            }
            result_ready = true;
        }

        {
            std::lock_guard<std::mutex> lock(worker_mutex);
            workers_finished++;
        }

        work_finished.notify_one();
    }
}

void Search::getFeatureUpdates(std::vector<int>& after_move_features, std::vector<int>& before_move_features, int king_square, const Move& move, const Board& board) {
    switch (move.type) {
        case (MoveType::Normal): {
            // Move piece
            const Piece& piece = board.getPiece(move.from);
            before_move_features.push_back(getFeature(move.from, piece, king_square));
            after_move_features.push_back(getFeature(move.to, piece, king_square));

            // Capture piece
            const Piece& captured_piece = board.getPiece(move.to);
            if (captured_piece.type != PieceType::None) {
                before_move_features.push_back(getFeature(move.to, captured_piece, king_square));
            }

            break;
        }
        case (MoveType::EnPassant): {
            // Move pawn
            const Piece& pawn = board.getPiece(move.from);
            before_move_features.push_back(getFeature(move.from, pawn, king_square));
            after_move_features.push_back(getFeature(move.to, pawn, king_square));

            // Capture piece
            int from_rank = move.from / 8;
            int to_rank = move.to / 8;
            int direction = to_rank - from_rank;
            int captured_square = move.to - (8 * direction);
            before_move_features.push_back(getFeature(captured_square, board.getPiece(captured_square), king_square));

            break;
        }
        case (MoveType::Castle): {
            // Move rook
            if (move.to == Square::C1) {
                // White Queen-side
                const Piece& rook = board.getPiece(Square::A1);
                before_move_features.push_back(getFeature(Square::A1, rook, king_square));
                after_move_features.push_back(getFeature(Square::D1, rook, king_square));
            } else if (move.to == Square::G1) {
                // White King-side
                const Piece& rook = board.getPiece(Square::H1);
                before_move_features.push_back(getFeature(Square::H1, rook, king_square));
                after_move_features.push_back(getFeature(Square::F1, rook, king_square));
            } else if (move.to == Square::C8) {
                // Black Queen-side
                const Piece& rook = board.getPiece(Square::A8);
                before_move_features.push_back(getFeature(Square::A8, rook, king_square));
                after_move_features.push_back(getFeature(Square::D8, rook, king_square));
            } else {
                // Black King-side
                const Piece& rook = board.getPiece(Square::H8);
                before_move_features.push_back(getFeature(Square::H8, rook, king_square));
                after_move_features.push_back(getFeature(Square::F8, rook, king_square));
            }
            break;
        }
        case (MoveType::Promotion): {
            // Promote pawn
            const Piece& pawn = board.getPiece(move.from);
            before_move_features.push_back(getFeature(move.from, pawn, king_square));
            after_move_features.push_back(getFeature(move.to, move.promotion_piece, king_square));

            // Capture piece
            const Piece& captured_piece = board.getPiece(move.to);
            if (captured_piece.type != PieceType::None) {
                before_move_features.push_back(getFeature(move.to, captured_piece, king_square));
            }
            
            break;
        }
    }
}

void Search::makeMove(const Move& move, Game& game, NNUE& nnue) {
    const Board& board = game.getBoard();
    const Piece& piece = board.getPiece(move.from);

    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            if (move.type == MoveType::Castle) {
                std::vector<int> black_added_features;
                std::vector<int> black_removed_features;
                getFeatureUpdates(black_added_features, black_removed_features, board.getKingSquare(Colour::Black), move, board);
                nnue.updateBlackAccumulator(black_added_features, black_removed_features);
            }
            
            game.makeMove(move);

            nnue.refreshWhiteAccumulator(getActiveFeatures(board, Colour::White));
        } else {
            if (move.type == MoveType::Castle) {
                std::vector<int> white_added_features;
                std::vector<int> white_removed_features;
                getFeatureUpdates(white_added_features, white_removed_features, board.getKingSquare(Colour::White), move, board);
                nnue.updateWhiteAccumulator(white_added_features, white_removed_features);
            }

            game.makeMove(move);

            nnue.refreshBlackAccumulator(getActiveFeatures(board, Colour::Black));
        }
    } else {
        std::vector<int> white_added_features;
        std::vector<int> white_removed_features;
        getFeatureUpdates(white_added_features, white_removed_features, board.getKingSquare(Colour::White), move, board);
        nnue.updateWhiteAccumulator(white_added_features, white_removed_features);

        std::vector<int> black_added_features;
        std::vector<int> black_removed_features;
        getFeatureUpdates(black_added_features, black_removed_features, board.getKingSquare(Colour::Black), move, board);
        nnue.updateBlackAccumulator(black_added_features, black_removed_features);

        game.makeMove(move);
    }
}

void Search::undoMove(const Move& move, Game& game, NNUE& nnue) {
    game.undoMove();

    const Board& board = game.getBoard();
    const Piece& piece = board.getPiece(move.from);

    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            if (move.type == MoveType::Castle) {
                std::vector<int> black_added_features;
                std::vector<int> black_removed_features;
                getFeatureUpdates(black_removed_features, black_added_features, board.getKingSquare(Colour::Black), move, board);
                nnue.updateBlackAccumulator(black_added_features, black_removed_features);
            }

            nnue.refreshWhiteAccumulator(getActiveFeatures(board, Colour::White));
        } else {
            if (move.type == MoveType::Castle) {
                std::vector<int> white_added_features;
                std::vector<int> white_removed_features;
                getFeatureUpdates(white_removed_features, white_added_features, board.getKingSquare(Colour::White), move, board);
                nnue.updateWhiteAccumulator(white_added_features, white_removed_features);
            }

            nnue.refreshBlackAccumulator(getActiveFeatures(board, Colour::Black));
        }
    } else {
        std::vector<int> white_added_features;
        std::vector<int> white_removed_features;
        getFeatureUpdates(white_removed_features, white_added_features, board.getKingSquare(Colour::White), move, board);
        nnue.updateWhiteAccumulator(white_added_features, white_removed_features);

        std::vector<int> black_added_features;
        std::vector<int> black_removed_features;
        getFeatureUpdates(black_removed_features, black_added_features, board.getKingSquare(Colour::Black), move, board);
        nnue.updateBlackAccumulator(black_added_features, black_removed_features);
    }
}

void Search::makeMove(const Move& move) {
    makeMove(move, game, nnue);
}

void Search::undoMove(const Move& move) {
    undoMove(move, game, nnue);
}

// void Search::makeBaseMove(const Move& move) {
//     makeMove(move);
    
    // {
    //     std::lock_guard<std::mutex> lock(worker_mutex);

    //     for (auto& [thread_game, thread_nnue] : thread_states) {
    //         makeMove(move, thread_game, thread_nnue);
    //     }
    // }
// }

// void Search::undoBaseMove(const Move& move) {
//     undoMove(move);

    // {
    //     std::lock_guard<std::mutex> lock(worker_mutex);

    //     for (auto& [thread_game, thread_nnue] : thread_states) {
    //         undoMove(move, thread_game, thread_nnue);
    //     }
    // }
// }

float Search::evaluate(Colour side_to_move, NNUE& nnue) const {
    return nnue.forward(static_cast<int>(side_to_move));
}

int Search::getFeature(int square, const Piece& piece, int king_square) const {
    int side;
    if (piece.colour == Colour::White) {
        side = 0;
    } else {
        side = 1;
    }

    int p_idx = static_cast<int>(piece.type) * 2 + side;
    int halfkp_idx = square + (p_idx + king_square * 10) * 64;

    return halfkp_idx;
}

std::vector<int> Search::getActiveFeatures(const Board& board, Colour colour) const {
    std::vector<int> active_features;

    int king_square = board.getKingSquare(colour);

    for (int square = 0; square < 64; square++) {
        const Piece& piece = board.getPiece(square);
        if (piece.type == PieceType::None || piece.type == PieceType::King) {
            continue;
        }

        active_features.push_back(getFeature(square, piece, king_square));
    }

    return active_features;
}

int Search::scoreMove(const Move& move, const Board& board, const Move* tt_move) const {
    if (tt_move != nullptr && move == *tt_move) {
        return 100000;
    }

    int score = 0;
    if (move.type == MoveType::Promotion) {
        score += 3000;
    }
    if (move.type == MoveType::Castle) {
        score += 100;
    }
    if (board.getPiece(move.to).type != PieceType::None) {
        const Piece& attacker = board.getPiece(move.from);
        const Piece& victim = board.getPiece(move.to);

        score += 1000 + (pieceValue(victim.type) * 10 - pieceValue(attacker.type));
    }

    return score;
}

void Search::orderMoves(std::vector<Move>& moves, Game& game, std::mt19937& rng) {
    // int sort_start = 0;

    // std::string position_key = game.getPositionFen();

    // auto it = previous_best_moves.find(position_key);
    // if (it != previous_best_moves.end()) {
    //     const Move& best_move = it->second;
    //     for (int i = 0; i < moves.size(); i++) {
    //         if (moves[i].from == best_move.from && moves[i].to == best_move.to && moves[i].type == best_move.type
    //                 && moves[i].promotion_piece.type == best_move.promotion_piece.type)
    //         {
    //             std::swap(moves[0], moves[i]);
    //             sort_start = 1;
    //             break;
    //         }
    //     }
    // }
    // std::sort(moves.begin() + sort_start, moves.end(), [&](const Move& a, const Move& b) {
    //     return (scoreMove(a, game.getBoard()) > scoreMove(b, game.getBoard()));
    // });


    TTEntry tt_entry;
    const Move* tt_move = nullptr;
    if (transposition_table.probe(game.getZobristKey(), tt_entry)) {
        tt_move = &tt_entry.best_move;
    }

    std::shuffle(moves.begin(), moves.end(), rng);

    std::stable_sort(moves.begin(), moves.end(),
        [&](const Move& a, const Move& b) {
            return scoreMove(a, game.getBoard(), tt_move) >
                scoreMove(b, game.getBoard(), tt_move);
        }
    );
}

// void Search::orderMoves(std::vector<Move>& moves, Game& game, std::mt19937& rng) {
//     struct ScoredMove {
//         Move move;
//         int score;
//     };

//     TTEntry tt_entry;
//     const Move* tt_move = nullptr;
//     if (transposition_table.probe(game.getZobristKey(), tt_entry)) {
//         tt_move = &tt_entry.best_move;
//     }

//     std::vector<ScoredMove> scored_moves;
//     scored_moves.reserve(moves.size());

//     for (const Move& move : moves) {
//         scored_moves.push_back({move, scoreMove(move, game.getBoard(), tt_move)});
//     }

//     std::shuffle(scored_moves.begin(), scored_moves.end(), rng);

//     std::stable_sort(scored_moves.begin(), scored_moves.end(),
//         [](const ScoredMove& a, const ScoredMove& b) {
//             return a.score > b.score;
//         }
//     );

//     for (std::size_t i = 0; i < moves.size(); ++i) {
//         moves[i] = scored_moves[i].move;
//     }
// }

int Search::pieceValue(PieceType type) const {
    switch (type) {
        case PieceType::Pawn:
            return 1;
        case PieceType::Knight:
            return 3;
        case PieceType::Bishop:
            return 3;
        case PieceType::Rook:
            return 5;
        case PieceType::Queen:
            return 9;
        case PieceType::King:
            return 10;
        case PieceType::None:
            return 0;
    }
    return 0;
}

void Search::reset() {
    nnue.refreshWhiteAccumulator(getActiveFeatures(game.getBoard(), Colour::White));
    nnue.refreshBlackAccumulator(getActiveFeatures(game.getBoard(), Colour::Black));

    // {
    //     std::lock_guard<std::mutex> lock(worker_mutex);

    //     for (auto& [thread_game, thread_nnue] : thread_states) {
    //         thread_game = this->game;

    //         thread_nnue.refreshWhiteAccumulator(getActiveFeatures(thread_game.getBoard(), Colour::White));
    //         thread_nnue.refreshBlackAccumulator(getActiveFeatures(thread_game.getBoard(), Colour::Black));
    //     }
    // }
}

Search::~Search() {
    running = false;

    work_available.notify_all();

    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}