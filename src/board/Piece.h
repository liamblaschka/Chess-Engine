#pragma once

enum class PieceType {
    None,
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King
};

enum class Colour {
    None,
    White,
    Black
};

struct Piece {
    PieceType type;
    Colour colour;

    Piece() : type(PieceType::None), colour(Colour::None) {}
    Piece(PieceType type, Colour colour) : type(type), colour(colour) {}
};
