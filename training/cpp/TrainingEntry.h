#pragma once

#include <vector>
#include <cstdint>

enum class PieceType : std::uint8_t {
    Pawn = 0,
    Knight = 1,
    Bishop = 2,
    Rook = 3,
    Queen = 4
};

enum class Side : std::uint8_t {
    White = 0,
    Black = 1
};

struct Piece {
    PieceType type;
    Side side;
    std::uint8_t square;
};

struct TrainingEntry {
    std::vector<Piece> pieces;
    std::uint8_t white_king_square;
    std::uint8_t black_king_square;
    Side side_to_move;
    float evaluation;
};