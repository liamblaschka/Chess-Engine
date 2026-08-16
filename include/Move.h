#pragma once

#include "Piece.h"
#include "CastleRights.h"

enum class MoveType {
    Normal,
    EnPassant,
    Castle,
    Promotion
};

struct Move {
    int from;
    int to;
    MoveType type;
    Piece promotion_piece;

    Move(int from_square, int to_square, MoveType type = MoveType::Normal, Piece promotion_piece = Piece())
        : from(from_square), to(to_square), type(type), promotion_piece(promotion_piece) {}

    Move(int from_rank, int from_file, int to_rank, int to_file, MoveType type = MoveType::Normal, Piece promotion_piece = Piece())
        : from(from_rank * 8 + from_file), to(to_rank * 8 + to_file), type(type), promotion_piece(promotion_piece) {}
};

struct MoveState {
    Move move;

    Piece captured_piece;
    int captured_square;

    CastleRights white_castle_rights;
    CastleRights black_castle_rights;
};
