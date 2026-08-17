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

    char getSymbol() const {
        char symbol;
        switch (type) {
            case PieceType::Pawn:
                symbol = 'p';
            case PieceType::Knight:
                symbol = 'n';
            case PieceType::Bishop:
                symbol = 'b';
            case PieceType::Rook:
                symbol = 'r';
            case PieceType::Queen:
                symbol = 'q';
            case PieceType::King:
                symbol = 'k';
            case PieceType::None:
                return ' ';
        }
        if (colour == Colour::White) {
            symbol += ('A' - 'a');
        }
        return symbol;
    }
};

Colour oppositeColour(Colour colour);
