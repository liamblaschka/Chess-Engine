#pragma once

#include "Piece.h"
#include "Move.h"
#include "CastleRights.h"
#include <array>
#include <vector>
#include <string>

class Board {
private:
    std::array<Piece, 64> squares;
    Colour turn;
    std::vector<MoveState> move_history;
    CastleRights white_castle_rights;
    CastleRights black_castle_rights;
public:
    Board();
    void makeMove(const Move& move);
    void undoMove();
    const MoveState* getLastMove() const;
    bool isSquareAttacked(int rank, int file, Colour opponent) const;
    bool isSquareAttacked(int square, Colour attacking_colour) const;
    bool isKingInCheck(Colour colour) const;
    bool isInsufficientMaterial() const;
    std::string getPositionKey(const std::vector<Move>& legal_moves) const;
    const Piece& getPiece(int square) const;
    const Piece& getPiece(int rank, int file) const;
    void setPiece(int square, Piece piece);
    void setPiece(int rank, int file, Piece piece);
    Colour getTurn() const;
    void setTurn(Colour colour);
    CastleRights getCastleRights(Colour colour) const;
    void setCastleRights(Colour colour, CastleRights rights);
    const std::array<Piece, 64>& getSquares() const;
    void clear();
    void draw();
};