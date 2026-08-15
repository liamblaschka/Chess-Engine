#include "MoveGenerator.h"
#include "Piece.h"
#include "Move.h"
#include "Board.h"
#include <vector>

MoveGenerator::MoveGenerator() {}

void MoveGenerator::generatePawnMoves(const Board& board, std::vector<Move>& moves, int rank, int file) {
    Colour turn = board.getTurn();
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

void MoveGenerator::generateKnightMoves(const Board& board, std::vector<Move>& moves, int rank, int file) {
    Colour turn = board.getTurn();

    int directions[8][2] = {
        {2, -1},
        {2, 1},
        {1, 2},
        {-1, 2},
        {-2, 1},
        {-2, -1},
        {-1, -2},
        {1, -2}
    };

    for (auto& direction : directions) {
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

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(const Board& board) {
    std::vector<Move> moves;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            const Piece& piece = board.getPiece(rank, file);
            if (piece.colour == board.getTurn()) {
                switch (piece.type) {
                    case (PieceType::Pawn):
                        generatePawnMoves(board, moves, rank, file);
                        break;
                    case (PieceType::Knight):
                        generateKnightMoves(board, moves, rank, file);
                        break;
                    case (PieceType::Bishop):
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