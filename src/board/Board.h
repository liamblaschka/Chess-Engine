#pragma once

#include "Piece.h"
#include <array>

class Board {
private:
    std::array<Piece, 64> squares;
    Colour turn;
public:
    Board();
    const Piece& getPiece(int square) const;
    const Piece& getPiece(int rank, int file) const;
    void setPiece(int rank, int file, Piece piece);
    Colour getTurn() const;
    void setTurn(Colour colour);
    void clear();
    void draw();
};