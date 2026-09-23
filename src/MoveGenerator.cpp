#include "MoveGenerator.h"
#include "Piece.h"
#include "Move.h"
#include "Board.h"
#include "SquareConstants.h"
#include "CastleRights.h"
#include <vector>
#include <cmath>

MoveGenerator::MoveGenerator() : en_passant_possible(false) {
    legal_moves.reserve(256);
    pseudo_legal_moves.reserve(256);
}

void MoveGenerator::generatePawnMoves(const Board& board, Colour turn, int rank, int file) {
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
        if (rank + direction > 0 && rank + direction < 7) {
            pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file));

            // Double move from starting rank
            if ((turn == Colour::White && rank == 1) || (turn == Colour::Black && rank == 6)) {
                if (board.getPiece(rank + (direction * 2), file).type == PieceType::None) {
                    pseudo_legal_moves.push_back(Move(rank, file, rank + (direction * 2), file));
                }
            }
        } else {
            pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file, MoveType::Promotion, Piece(PieceType::Knight, turn)));
            pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file, MoveType::Promotion, Piece(PieceType::Bishop, turn)));
            pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file, MoveType::Promotion, Piece(PieceType::Rook, turn)));
            pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file, MoveType::Promotion, Piece(PieceType::Queen, turn)));
        }
    }

    // Capture toward a-file
    if (file - 1 >= 0) {
        const Piece& target = board.getPiece(rank + direction, file - 1);
        if (target.colour != Colour::None && target.colour != turn) {
            if (rank + direction > 0 && rank + direction < 7) {
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file - 1));
            } else {
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file - 1, MoveType::Promotion, Piece(PieceType::Knight, turn)));
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file - 1, MoveType::Promotion, Piece(PieceType::Bishop, turn)));
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file - 1, MoveType::Promotion, Piece(PieceType::Rook, turn)));
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file - 1, MoveType::Promotion, Piece(PieceType::Queen, turn)));
            }
        }
    }
    
    // Capture toward h-file
    if (file + 1 < 8) {
        const Piece& target = board.getPiece(rank + direction, file + 1);
        if (target.colour != Colour::None && target.colour != turn) {
            if (rank + direction > 0 && rank + direction < 7) {
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file + 1));
            } else {
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file + 1, MoveType::Promotion, Piece(PieceType::Knight, turn)));
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file + 1, MoveType::Promotion, Piece(PieceType::Bishop, turn)));
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file + 1, MoveType::Promotion, Piece(PieceType::Rook, turn)));
                pseudo_legal_moves.push_back(Move(rank, file, rank + direction, file + 1, MoveType::Promotion, Piece(PieceType::Queen, turn)));
            }
        }
    }

    // En passant
    int en_passant_square = board.getEnPassantSquare();
    if (en_passant_square != -1) {
        int en_passant_rank = en_passant_square / 8;
        int en_passant_file = en_passant_square % 8;
        if (en_passant_rank == rank + direction && std::abs(en_passant_file - file) == 1) {
            pseudo_legal_moves.push_back(Move(rank, file, en_passant_rank, en_passant_file, MoveType::EnPassant));
        }
    }
}

void MoveGenerator::generateKnightMoves(const Board& board, Colour turn, int rank, int file) {
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
            pseudo_legal_moves.push_back(Move(rank, file, target_rank, target_file));
        }
    }
}

void MoveGenerator::generateSlidingMoves(const Board& board, Colour turn, int rank, int file, const int directions[][2], int direction_count) {
    for (int i = 0; i < direction_count; i++) {
        int rank_direction = directions[i][0];
        int file_direction = directions[i][1];

        int target_rank = rank + rank_direction;
        int target_file = file + file_direction;
        while (target_rank >= 0 && target_file >= 0 && target_rank < 8 && target_file < 8) {
            const Piece& target = board.getPiece(target_rank, target_file);
            if (target.type == PieceType::None) {
                pseudo_legal_moves.push_back(Move(rank, file, target_rank, target_file));
            } else {
                if (target.colour != turn) {
                    pseudo_legal_moves.push_back(Move(rank, file, target_rank, target_file));
                }
                break;
            }

            target_rank += rank_direction;
            target_file += file_direction; 
        }
    }
}

void MoveGenerator::generateBishopMoves(const Board& board, Colour turn, int rank, int file) {
    const int directions[4][2] = {
        {1, -1},
        {1, 1},
        {-1, 1},
        {-1, -1}
    };

    generateSlidingMoves(board, turn, rank, file, directions, 4);
}

void MoveGenerator::generateRookMoves(const Board& board, Colour turn, int rank, int file) {
    const int directions[4][2] = {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1}
    };

    generateSlidingMoves(board, turn, rank, file, directions, 4);
}

void MoveGenerator::generateQueenMoves(const Board& board, Colour turn, int rank, int file) {
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

    generateSlidingMoves(board, turn, rank, file, directions, 8);
}

void MoveGenerator::generateKingMoves(const Board& board, Colour turn, int rank, int file) {
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
            pseudo_legal_moves.push_back(Move(rank, file, target_rank, target_file));
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
                    pseudo_legal_moves.push_back(Move(Square::E1, Square::C1, MoveType::Castle));
                }
            } else {
                if (board.getPiece(Square::B8).type == PieceType::None
                    && board.getPiece(Square::C8).type == PieceType::None
                    && board.getPiece(Square::D8).type == PieceType::None
                    && !board.isSquareAttacked(Square::D8, opponent))
                {
                    pseudo_legal_moves.push_back(Move(Square::E8, Square::C8, MoveType::Castle));
                }
            }
        }
        if (castle_rights.king_side) {
            if (turn == Colour::White) {
                if (board.getPiece(Square::F1).type == PieceType::None
                    && board.getPiece(Square::G1).type == PieceType::None
                    && !board.isSquareAttacked(Square::F1, opponent))
                {
                    pseudo_legal_moves.push_back(Move(Square::E1, Square::G1, MoveType::Castle));
                }
            } else {
                if (board.getPiece(Square::F8).type == PieceType::None
                    && board.getPiece(Square::G8).type == PieceType::None
                    && !board.isSquareAttacked(Square::F8, opponent))
                {
                    pseudo_legal_moves.push_back(Move(Square::E8, Square::G8, MoveType::Castle));
                }
            }
        }
    }
}

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(const Board& board) {
    Colour turn = board.getTurn();
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            const Piece& piece = board.getPiece(rank, file);
            if (piece.colour == turn) {
                switch (piece.type) {
                    case (PieceType::Pawn):
                        generatePawnMoves(board, turn, rank, file);
                        break;
                    case (PieceType::Knight):
                        generateKnightMoves(board, turn, rank, file);
                        break;
                    case (PieceType::Bishop):
                        generateBishopMoves(board, turn, rank, file);
                        break;
                    case (PieceType::Rook):
                        generateRookMoves(board, turn, rank, file);
                        break;
                    case (PieceType::Queen):
                        generateQueenMoves(board, turn, rank, file);
                        break;
                    case (PieceType::King):
                        generateKingMoves(board, turn, rank, file);
                        break;
                    case (PieceType::None):
                        break;
                }
            }
        }
    }

    return pseudo_legal_moves;
}

void MoveGenerator::generateLegalMoves(Board& board) {
    legal_moves.clear();
    pseudo_legal_moves.clear();
    en_passant_possible = false;

    pseudo_legal_moves = generatePseudoLegalMoves(board);

    const Colour turn = board.getTurn();

    for (const auto& move : pseudo_legal_moves) {
        board.makeMove(move);

        if (!board.isKingInCheck(turn)) {
            legal_moves.push_back(move);

            if (move.type == MoveType::EnPassant) {
                en_passant_possible = true;
            }
        }

        board.undoMove();
    }
}

const std::vector<Move>& MoveGenerator::getLegalMoves() const { return legal_moves; }

bool MoveGenerator::isEnPassantPossible() const { return en_passant_possible; }