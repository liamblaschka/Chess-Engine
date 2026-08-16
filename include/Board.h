#pragma once

#include "Piece.h"
#include "Move.h"
#include <array>

class Board {
private:
    std::array<Piece, 64> squares;
    Colour turn;
    std::vector<MoveState> move_history;
public:
    Board();
    void makeMove(const Move& move);
    void undoMove();
    const MoveState* getLastMove() const;
    bool isKingInCheck(Colour colour) const;
    const Piece& getPiece(int square) const;
    const Piece& getPiece(int rank, int file) const;
    void setPiece(int rank, int file, Piece piece);
    Colour getTurn() const;
    void setTurn(Colour colour);
    void clear();
    void draw();
};