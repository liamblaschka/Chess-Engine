#include "Search.h"
#include "Game.h"
#include "Board.h"
#include "Move.h"
#include "SquareConstants.h"
#include "NNUE.h"
#include <array>
#include <vector>
#include <limits>
#include <algorithm>
#include <utility>

Search::Search() : nnue("nnue.bin") {}

float Search::maximise(Game& game, int depth, float alpha, float beta) {
    float best_score = std::numeric_limits<float>::lowest();
    Move best_move;

    std::vector<Move> moves = game.getLegalMoves();
    orderMoves(moves, game.getBoard());
    GameState game_state = game.getGameState(moves);
    if (game_state == GameState::Draw) {
        return DRAW_SCORE;
    }
    if (game_state == GameState::Checkmate) {
        return -CHECKMATE_SCORE - depth;
    }

    if (depth == 0) {
        return evaluate(game.getBoard());
    }

    for (const Move& move : moves) {
        makeMove(move, game);
        float score = minimise(game, depth - 1, alpha, beta);
        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
        undoMove(move, game);

        if (best_score >= beta) {
            break;
        }
        alpha = std::max(alpha, best_score);
    }

    previous_best_moves[game.getBoard().getPositionKey(moves)] = best_move;

    return best_score;
}

float Search::minimise(Game& game, int depth, float alpha, float beta) {
    float best_score = std::numeric_limits<float>::max();
    Move best_move;

    std::vector<Move> moves = game.getLegalMoves();
    orderMoves(moves, game.getBoard());
    GameState game_state = game.getGameState(moves);
    if (game_state == GameState::Draw) {
        return DRAW_SCORE;
    }
    if (game_state == GameState::Checkmate) {
        return CHECKMATE_SCORE + depth;
    }

    if (depth == 0) {
        return evaluate(game.getBoard());
    }

    for (const Move& move : moves) {
        makeMove(move, game);
        float score = maximise(game, depth - 1, alpha, beta);
        if (score < best_score) {
            best_score = score;
            best_move = move;
        }
        undoMove(move, game);

        if (best_score <= alpha) {
            break;
        }
        beta = std::min(beta, best_score);
    }

    previous_best_moves[game.getBoard().getPositionKey(moves)] = best_move;

    return best_score;
}

Move Search::minimax(Game& game) {
    nnue.refreshWhiteAccumulator(white_acc_values, getActiveFeatures(game.getBoard(), Colour::White));
    nnue.refreshBlackAccumulator(black_acc_values, getActiveFeatures(game.getBoard(), Colour::Black));

    Move best_move;
    std::vector<Move> moves = game.getLegalMoves();
    orderMoves(moves, game.getBoard());
    float alpha = std::numeric_limits<float>::lowest();
    float beta = std::numeric_limits<float>::max();

    int depth = 6;

    if (game.getTurn() == Colour::White) {
        float best_score = std::numeric_limits<float>::lowest();
        for (const Move& move : moves) {
            makeMove(move, game);

            float score = minimise(game, depth - 1, alpha, beta);
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
            alpha = std::max(alpha, best_score);

            undoMove(move, game);
        }
    } else {
        float best_score = std::numeric_limits<float>::max();
        for (const Move& move : moves) {
            makeMove(move, game);

            float score = maximise(game, depth - 1, alpha, beta);
            if (score < best_score) {
                best_score = score;
                best_move = move;
            }
            beta = std::min(beta, best_score);

            undoMove(move, game);
        }
    }

    return best_move;
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

void Search::makeMove(const Move& move, Game& game) {
    const Board& board = game.getBoard();
    const Piece& piece = board.getPiece(move.from);

    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            if (move.type == MoveType::Castle) {
                std::vector<int> black_added_features;
                std::vector<int> black_removed_features;
                getFeatureUpdates(black_added_features, black_removed_features, board.getKingSquare(Colour::Black), move, board);
                nnue.updateBlackAccumulator(black_acc_values, black_added_features, black_removed_features);
            }
            
            game.makeMove(move);

            nnue.refreshWhiteAccumulator(white_acc_values, getActiveFeatures(board, Colour::White));
        } else {
            if (move.type == MoveType::Castle) {
                std::vector<int> white_added_features;
                std::vector<int> white_removed_features;
                getFeatureUpdates(white_added_features, white_removed_features, board.getKingSquare(Colour::White), move, board);
                nnue.updateWhiteAccumulator(white_acc_values, white_added_features, white_removed_features);
            }

            game.makeMove(move);

            nnue.refreshBlackAccumulator(black_acc_values, getActiveFeatures(board, Colour::Black));
        }
    } else {
        std::vector<int> white_added_features;
        std::vector<int> white_removed_features;
        getFeatureUpdates(white_added_features, white_removed_features, board.getKingSquare(Colour::White), move, board);
        nnue.updateWhiteAccumulator(white_acc_values, white_added_features, white_removed_features);

        std::vector<int> black_added_features;
        std::vector<int> black_removed_features;
        getFeatureUpdates(black_added_features, black_removed_features, board.getKingSquare(Colour::Black), move, board);
        nnue.updateBlackAccumulator(black_acc_values, black_added_features, black_removed_features);

        game.makeMove(move);
    }
}

void Search::undoMove(const Move& move, Game& game) {
    game.undoMove();

    const Board& board = game.getBoard();
    const Piece& piece = board.getPiece(move.from);

    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            if (move.type == MoveType::Castle) {
                std::vector<int> black_added_features;
                std::vector<int> black_removed_features;
                getFeatureUpdates(black_removed_features, black_added_features, board.getKingSquare(Colour::Black), move, board);
                nnue.updateBlackAccumulator(black_acc_values, black_added_features, black_removed_features);
            }

            nnue.refreshWhiteAccumulator(white_acc_values, getActiveFeatures(board, Colour::White));
        } else {
            if (move.type == MoveType::Castle) {
                std::vector<int> white_added_features;
                std::vector<int> white_removed_features;
                getFeatureUpdates(white_removed_features, white_added_features, board.getKingSquare(Colour::White), move, board);
                nnue.updateWhiteAccumulator(white_acc_values, white_added_features, white_removed_features);
            }

            nnue.refreshBlackAccumulator(black_acc_values, getActiveFeatures(board, Colour::Black));
        }
    } else {
        std::vector<int> white_added_features;
        std::vector<int> white_removed_features;
        getFeatureUpdates(white_removed_features, white_added_features, board.getKingSquare(Colour::White), move, board);
        nnue.updateWhiteAccumulator(white_acc_values, white_added_features, white_removed_features);

        std::vector<int> black_added_features;
        std::vector<int> black_removed_features;
        getFeatureUpdates(black_removed_features, black_added_features, board.getKingSquare(Colour::Black), move, board);
        nnue.updateBlackAccumulator(black_acc_values, black_added_features, black_removed_features);
    }
}

float Search::evaluate(const Board& board) {
    int side_to_move;
    if (board.getTurn() == Colour::White) {
        side_to_move = 0;
    } else {
        side_to_move = 1;
    }

    float score = nnue.forward(white_acc_values, black_acc_values, side_to_move);
    return score;
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

int Search::scoreMove(const Move& move, const Board& board) const {
    int score = 0;
    if (move.type == MoveType::Promotion) {
        score += 3000;
    }
    if (move.type == MoveType::Castle) {
        score += 100;
    }
    if (board.getPiece(move.to).type != PieceType::None) {
        score += 1000;
    }

    return score;
}

void Search::orderMoves(std::vector<Move>& moves, const Board& board) {
    int sort_start = 0;

    std::string position_key = board.getPositionKey(moves);

    auto it = previous_best_moves.find(position_key);
    if (it != previous_best_moves.end()) {
        const Move& best_move = it->second;
        for (int i = 0; i < moves.size(); i++) {
            if (moves[i].from == best_move.from && moves[i].to == best_move.to && moves[i].type == best_move.type
                    && moves[i].promotion_piece.type == best_move.promotion_piece.type)
            {
                std::swap(moves[0], moves[i]);
                sort_start = 1;
                break;
            }
        }
    }
    std::sort(moves.begin() + sort_start, moves.end(), [&](const Move& a, const Move& b) {
        return (scoreMove(a, board) > scoreMove(b, board));
    });
}

// int Search::pieceValue(PieceType piece_type) const {
//     switch (piece_type) {
//         case (PieceType::Pawn):
//             return 100;
//         case (PieceType::Knight):
//             return 300;
//         case (PieceType::Bishop):
//             return 300;
//         case (PieceType::Rook):
//             return 500;
//         case (PieceType::Queen):
//             return 900;
//         default:
//             return 0;
//     }
// }

// int Search::evaluate(const Board& board) const {
//     int score = 0;

//     for (int rank = 0; rank < 8; rank++) {
//         for (int file = 0; file < 8; file++) {
//             const Piece& piece = board.getPiece(rank, file);
//             int value = pieceValue(piece.type);
//             if (piece.colour == Colour::White) {
//                 score += value;
//             } else {
//                 score -= value;
//             }

//             if (piece.type == PieceType::Knight || piece.type == PieceType::Bishop) {
//                 if (piece.colour == Colour::White) {
//                     if (rank == 0) {
//                         score -= 50;
//                     }
//                 } else {
//                     if (rank == 7) {
//                         score += 50;
//                     }
//                 }
//             } else if (piece.type == PieceType::Pawn) {
//                 if (piece.colour == Colour::White) {
//                     if (file == 3 || file == 4) {
//                         if (rank == 1) {
//                             score -= 30;
//                         } else if (rank == 2) {
//                             score += 10;
//                         } else if (rank == 3) {
//                             score += 25;
//                         }
//                     }
//                     score += (5 * (rank - 1));
                    
//                     const int directions[2] = {1, -1};
//                     for (const auto& direction : directions) {
//                         int pawn_file = file + direction;
//                         if (pawn_file >= 0 && pawn_file < 8) {
//                             for (int pawn_rank = rank - 1; pawn_rank <= rank + 1; pawn_rank++) {
//                                 if (pawn_rank >= 0 && pawn_rank < 8) {
//                                     const Piece& pawn_piece = board.getPiece(pawn_rank, pawn_file);
//                                     if (pawn_piece.type == PieceType::Pawn && pawn_piece.colour == Colour::White) {
//                                         score += 10;
//                                         break;
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                     for (int pawn_rank = 1; pawn_rank < 8; pawn_rank++) {
//                         const Piece& pawn_piece = board.getPiece(pawn_rank, file);
//                         if (pawn_piece.type == PieceType::Pawn && pawn_piece.colour == Colour::White) {
//                             score -= 10;
//                         }
//                     }
                    
//                 } else {
//                     if (file == 3 || file == 4) {
//                         if (rank == 6) {
//                             score += 30;
//                         } else if (rank == 5) {
//                             score -= 10;
//                         } else if (rank == 4) {
//                             score -= 25;
//                         }
//                     }
//                     score -= (5 * (6 - rank));

//                     const int directions[2] = {1, -1};
//                     for (const auto& direction : directions) {
//                         int pawn_file = file + direction;
//                         if (pawn_file >= 0 && pawn_file < 8) {
//                             for (int pawn_rank = rank - 1; pawn_rank <= rank + 1; pawn_rank++) {
//                                 if (pawn_rank >= 0 && pawn_rank < 8) {
//                                     const Piece& pawn_piece = board.getPiece(pawn_rank, pawn_file);
//                                     if (pawn_piece.type == PieceType::Pawn && pawn_piece.colour == Colour::Black) {
//                                         score -= 10;
//                                         break;
//                                     }
//                                 }
//                             }
//                         }
//                     }
//                     for (int pawn_rank = 1; pawn_rank < 8; pawn_rank++) {
//                         const Piece& pawn_piece = board.getPiece(pawn_rank, file);
//                         if (pawn_piece.type == PieceType::Pawn && pawn_piece.colour == Colour::Black) {
//                             score += 10;
//                         }
//                     }
//                 }
            
//             // King safety
//             } else if (piece.type == PieceType::King) {
//                 if (piece.colour == Colour::White) {
//                     if (rank == 0) {
//                         for (int i = -1; i <= 1; i++) {
//                             if (file + i >= 0 && file + i < 8) {
//                                 const Piece& protecting_piece = board.getPiece(1, file + i);
//                                 if (protecting_piece.colour == Colour::White && protecting_piece.type == PieceType::Pawn) {
//                                     score += 25;
//                                 }
//                             } else {
//                                 score += 25;
//                             }
//                         }
//                     }
//                 } else {
//                     if (rank == 7) {
//                         for (int i = -1; i <= 1; i++) {
//                             if (file + i >= 0 && file + i < 8) {
//                                 const Piece& protecting_piece = board.getPiece(6, file + i);
//                                 if (protecting_piece.colour == Colour::Black && protecting_piece.type == PieceType::Pawn) {
//                                     score -= 25;
//                                 }
//                             } else {
//                                 score -= 25;
//                             }
//                         }
//                     }
//                 }

//             // Rooks connected
//             // add preference for rook on file attacking important pieces, and for queen above rook,
//             } else if (piece.type == PieceType::Rook) {
//                 const int directions[2][2] = {
//                     {1, 0},
//                     {0, 1}
//                 };
//                 for (const auto& direction : directions) {
//                     int rank_direction = direction[0];
//                     int file_direction = direction[1];

//                     int current_rank = rank + rank_direction;
//                     int current_file = file + file_direction;
//                     while (current_rank >= 0 && current_file >= 0 && current_rank < 8 && current_file < 8) {
//                         const Piece& current_piece = board.getPiece(current_rank, current_file);
//                         if (current_piece.colour == piece.colour && current_piece.type == PieceType::Rook) {
//                             if (piece.colour == Colour::White) {
//                                 if (rank_direction == 1) {
//                                     score += 50;
//                                 } else {
//                                     score += 25;
//                                 }
//                             } else {
//                                 if (rank_direction == 1) {
//                                     score -= 50;
//                                 } else {
//                                     score -= 25;
//                                 }
//                             }
//                         } else if (current_piece.type != PieceType::None) {
//                             break;
//                         }

//                         current_rank += rank_direction;
//                         current_file += file_direction; 
//                     }
//                 }
//             } else if (piece.type == PieceType::Queen) {
//                 const int directions[2][2] = {
//                     {1, 0},
//                     {-1, 0}
//                 };
//                 for (const auto& direction : directions) {
//                     int rank_direction = direction[0];
//                     int current_rank = rank + rank_direction;
//                     while (current_rank >= 0 && current_rank < 8) {
//                         const Piece& current_piece = board.getPiece(current_rank, file);
//                         if (current_piece.colour == piece.colour && current_piece.type == PieceType::Rook) {
//                             if (piece.colour == Colour::White) {
//                                 score += 25;
//                             } else {
//                                 score -= 25;
//                             }
//                         } else if (current_piece.type != PieceType::None) {
//                             break;
//                         }

//                         current_rank += rank_direction;
//                     }
//                 }
//             }
//         }
//     }

//     return score;
// }
