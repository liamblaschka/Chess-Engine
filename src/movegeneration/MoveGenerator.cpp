#include "MoveGenerator.h"
#include "Piece.h"
#include "Move.h"
#include "Board.h"
#include <vector>

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

void MoveGenerator::generateBishopMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file) {
    const int directions[4][2] = {
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
                        break;
                    case (PieceType::Queen):
                        break;
                    case (PieceType::King):
                        break;
                    case (PieceType::None):
                        break;
                }
            }
        }
    }

    return moves;
}