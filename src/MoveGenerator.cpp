#include "MoveGenerator.h"
#include "Piece.h"
#include "Move.h"
#include "Board.h"
#include "SquareConstants.h"
#include "CastleRights.h"
#include <vector>
#include <cmath>

MoveGenerator::MoveGenerator() {}

void MoveGenerator::generatePawnMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    int direction;
    if (turn == Colour::White) {
        direction = 1;
    } else {
        direction = -1;
    }

    if (rank + direction < 0 || rank + direction >= 8) {
        return;
    }

    // Foward move
    if (board.getPiece(rank + direction, file).type == PieceType::None) {
        moves.push_back(Move(rank, file, rank + direction, file));

        // Double move from starting rank
        if ((turn == Colour::White && rank == 1) || (turn == Colour::Black && rank == 6)) {
            if (board.getPiece(rank + (direction * 2), file).type == PieceType::None) {
                moves.push_back(Move(rank, file, rank + (direction * 2), file));
            }
        }
    }

    // Capture toward a-file
    if (file - 1 >= 0) {
        const Piece& target = board.getPiece(rank + direction, file - 1);
        if (target.colour != Colour::None && target.colour != turn) {
            moves.push_back(Move(rank, file, rank + direction, file - 1));
        }
    }
    
    // Capture toward h-file
    if (file + 1 < 8) {
        const Piece& target = board.getPiece(rank + direction, file + 1);
        if (target.colour != Colour::None && target.colour != turn) {
            moves.push_back(Move(rank, file, rank + direction, file + 1));
        }
    }

    // En passant
    const MoveState* last_move = board.getLastMove();
    if (last_move != nullptr) {
        int from_rank = last_move->move.from / 8;
        int to_rank = last_move->move.to / 8;
        int to_file = last_move->move.to % 8;
        const Piece& last_move_piece = board.getPiece(last_move->move.to);
        if (last_move_piece.type == PieceType::Pawn && std::abs(to_rank - from_rank) == 2) {
            if (to_rank == rank) {
                if (to_file == file - 1) {
                    moves.push_back(Move(rank, file, rank + direction, file - 1, MoveType::EnPassant));
                } else if (to_file == file + 1) {
                    moves.push_back(Move(rank, file, rank + direction, file + 1, MoveType::EnPassant));
                }
            }
        }
    }
}

void MoveGenerator::generateKnightMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    const int directions[8][2] = {
        {2, -1},
        {2, 1},
        {1, 2},
        {-1, 2},
        {-2, 1},
        {-2, -1},
        {-1, -2},
        {1, -2}
    };

    for (const auto& direction : directions) {
        int target_rank = rank + direction[0];
        int target_file = file + direction[1];

        if (target_rank < 0 || target_rank >= 8 || target_file < 0 || target_file >= 8) {
            continue;
        }

        const Piece& target = board.getPiece(target_rank, target_file);
        if (target.type == PieceType::None || target.colour != turn) {
            moves.push_back(Move(rank, file, target_rank, target_file));
        }
    }
}

void MoveGenerator::generateSlidingMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file, const int directions[][2], int direction_count) {
    for (int i = 0; i < direction_count; i++) {
        int rank_direction = directions[i][0];
        int file_direction = directions[i][1];

        int target_rank = rank + rank_direction;
        int target_file = file + file_direction;
        while (target_rank >= 0 && target_file >= 0 && target_rank < 8 && target_file < 8) {
            const Piece& target = board.getPiece(target_rank, target_file);
            if (target.type == PieceType::None) {
                moves.push_back(Move(rank, file, target_rank, target_file));
            } else {
                if (target.colour != turn) {
                    moves.push_back(Move(rank, file, target_rank, target_file));
                }
                break;
            }

            target_rank += rank_direction;
            target_file += file_direction; 
        }
    }
}

void MoveGenerator::generateBishopMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    const int directions[4][2] = {
        {1, -1},
        {1, 1},
        {-1, 1},
        {-1, -1}
    };

    generateSlidingMoves(board, moves, turn, rank, file, directions, 4);
}

void MoveGenerator::generateRookMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    const int directions[4][2] = {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1}
    };

    generateSlidingMoves(board, moves, turn, rank, file, directions, 4);
}

void MoveGenerator::generateQueenMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    const int directions[8][2] = {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1},
        {1, -1},
        {1, 1},
        {-1, 1},
        {-1, -1}
    };

    generateSlidingMoves(board, moves, turn, rank, file, directions, 8);
}

void MoveGenerator::generateKingMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    const int directions[8][2] = {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1},
        {1, -1},
        {1, 1},
        {-1, 1},
        {-1, -1}
    };

    for (const auto& direction : directions) {
        int rank_direction = direction[0];
        int file_direction = direction[1];

        int target_rank = rank + rank_direction;
        int target_file = file + file_direction;

        if (target_rank < 0 || target_rank >= 8 || target_file < 0 || target_file >= 8) {
            continue;
        }

        const Piece& target = board.getPiece(target_rank, target_file);
        if (target.type == PieceType::None || target.colour != turn) {
            moves.push_back(Move(rank, file, target_rank, target_file));
        }
    }

    // Castle
    if (!board.isKingInCheck(turn)) {
        Colour opponent = oppositeColour(turn);
        CastleRights castle_rights = board.getCastleRights(turn);
        if (castle_rights.queen_side) {
            if (turn == Colour::White) {
                if (board.getPiece(Square::B1).type == PieceType::None
                    && board.getPiece(Square::C1).type == PieceType::None
                    && board.getPiece(Square::D1).type == PieceType::None
                    && !board.isSquareAttacked(Square::D1, opponent))
                {
                    moves.push_back(Move(Square::E1, Square::C1, MoveType::Castle));
                }
            } else {
                if (board.getPiece(Square::B8).type == PieceType::None
                    && board.getPiece(Square::C8).type == PieceType::None
                    && board.getPiece(Square::D8).type == PieceType::None
                    && !board.isSquareAttacked(Square::D8, opponent))
                {
                    moves.push_back(Move(Square::E8, Square::C8, MoveType::Castle));
                }
            }
        }
        if (castle_rights.king_side) {
            if (turn == Colour::White) {
                if (board.getPiece(Square::F1).type == PieceType::None
                    && board.getPiece(Square::G1).type == PieceType::None
                    && !board.isSquareAttacked(Square::F1, opponent))
                {
                    moves.push_back(Move(Square::E1, Square::G1, MoveType::Castle));
                }
            } else {
                if (board.getPiece(Square::F8).type == PieceType::None
                    && board.getPiece(Square::G8).type == PieceType::None
                    && !board.isSquareAttacked(Square::F8, opponent))
                {
                    moves.push_back(Move(Square::E8, Square::G8, MoveType::Castle));
                }
            }
        }
    }
}

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(const Board& board) {
    std::vector<Move> moves;
    Colour turn = board.getTurn();
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            const Piece& piece = board.getPiece(rank, file);
            if (piece.colour == turn) {
                switch (piece.type) {
                    case (PieceType::Pawn):
                        generatePawnMoves(board, moves, turn, rank, file);
                        break;
                    case (PieceType::Knight):
                        generateKnightMoves(board, moves, turn, rank, file);
                        break;
                    case (PieceType::Bishop):
                        generateBishopMoves(board, moves, turn, rank, file);
                        break;
                    case (PieceType::Rook):
                        generateRookMoves(board, moves, turn, rank, file);
                        break;
                    case (PieceType::Queen):
                        generateQueenMoves(board, moves, turn, rank, file);
                        break;
                    case (PieceType::King):
                        generateKingMoves(board, moves, turn, rank, file);
                        break;
                    case (PieceType::None):
                        break;
                }
            }
        }
    }

    return moves;
}

std::vector<Move> MoveGenerator::generateLegalMoves(Board& board) {
    std::vector<Move> legal_moves;
    std::vector<Move> pseudo_legal_moves = generatePseudoLegalMoves(board);

    const Colour turn = board.getTurn();

    for (const auto& move : pseudo_legal_moves) {
        board.makeMove(move);

        if (!board.isKingInCheck(turn)) {
            legal_moves.push_back(move);
        }

        board.undoMove();
    }

    return legal_moves;
}
