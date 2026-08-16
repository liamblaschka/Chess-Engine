#pragma once

#include "Piece.h"

struct Move {
    int from;
    int to;

    Move(int from_rank, int from_file, int to_rank, int to_file)
        : from(from_rank * 8 + from_file), to(to_rank * 8 + to_file) {}
};

struct MoveState {
    Move move;
    Piece captured_piece;
};
