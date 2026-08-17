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
                break;
            case PieceType::Knight:
                symbol = 'n';
                break;
            case PieceType::Bishop:
                symbol = 'b';
                break;
            case PieceType::Rook:
                symbol = 'r';
                break;
            case PieceType::Queen:
                symbol = 'q';
                break;
            case PieceType::King:
                symbol = 'k';
                break;
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
