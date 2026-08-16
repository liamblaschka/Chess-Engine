#pragma once

#include "Piece.h"

enum class MoveType {
    Normal,
    EnPassant
};

struct Move {
    int from;
    int to;
    MoveType type;

    Move(int from_rank, int from_file, int to_rank, int to_file, MoveType type = MoveType::Normal)
        : from(from_rank * 8 + from_file), to(to_rank * 8 + to_file), type(type) {}
};

struct MoveState {
    Move move;
    Piece captured_piece;
    int captured_square;
};
