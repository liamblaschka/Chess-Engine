#include "Search.h"
#include "Game.h"
#include "Board.h"
#include "Move.h"
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
        game.makeMove(move);
        float score = minimise(game, depth - 1, alpha, beta);
        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
        game.undoMove();

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
        game.makeMove(move);
        float score = maximise(game, depth - 1, alpha, beta);
        if (score < best_score) {
            best_score = score;
            best_move = move;
        }
        game.undoMove();

        if (best_score <= alpha) {
            break;
        }
        beta = std::min(beta, best_score);
    }

    previous_best_moves[game.getBoard().getPositionKey(moves)] = best_move;

    return best_score;
}

Move Search::minimax(Game& game) {
    Move best_move;
    std::vector<Move> moves = game.getLegalMoves();
    orderMoves(moves, game.getBoard());
    float alpha = std::numeric_limits<float>::lowest();
    float beta = std::numeric_limits<float>::max();

    int depth = 5;
    // int pieces_count = game.getBoard().countPieces();
    // if (pieces_count <= 16) {
    //     depth = 5;
    // }
    // if (pieces_count <= 8) {
    //     depth = 6;
    // }
    // if (pieces_count <= 4) {
    //     depth = 7;
    // }

    if (game.getTurn() == Colour::White) {
        float best_score = std::numeric_limits<float>::lowest();
        for (const Move& move : moves) {
            game.makeMove(move);

            float score = minimise(game, depth - 1, alpha, beta);
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }
            alpha = std::max(alpha, best_score);

            game.undoMove();
        }
    } else {
        float best_score = std::numeric_limits<float>::max();
        for (const Move& move : moves) {
            game.makeMove(move);

            float score = maximise(game, depth - 1, alpha, beta);
            if (score < best_score) {
                best_score = score;
                best_move = move;
            }
            beta = std::min(beta, best_score);

            game.undoMove();
        }
    }

    return best_move;
}

float Search::evaluate(const Board& board) {
    std::vector<int> active_features;
    for (int square = 0; square < 64; square++) {
        const Piece& piece = board.getPiece(square);
        if (piece.type == PieceType::None) {
            continue;
        }

        int side;
        if (piece.colour == Colour::White) {
            side = 0;
        } else {
            side = 1;
        }

        int i = side * 64 * 6 + static_cast<int>(piece.type) * 64 + square;
        active_features.push_back(i);
    }

    return nnue.forward(active_features);
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
