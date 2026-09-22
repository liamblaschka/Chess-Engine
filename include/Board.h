#pragma once

#include "Piece.h"
#include "Move.h"
#include "CastleRights.h"
#include <array>
#include <vector>
#include <string>
#include <cstdint>

class Board {
private:
    std::array<Piece, 64> squares;
    Colour turn;
    std::vector<MoveState> move_history;

    int white_king_square;
    int black_king_square;

    CastleRights white_castle_rights;
    CastleRights black_castle_rights;

    int en_passant_square = -1;

    std::uint64_t zobrist_key;

    std::uint64_t calculateZobristKey();

public:
    Board();
    void makeMove(const Move& move);
    void undoMove();
    const MoveState* getLastMove() const;
    bool isSquareAttacked(int rank, int file, Colour opponent) const;
    bool isSquareAttacked(int square, Colour attacking_colour) const;
    int getKingSquare(Colour colour) const;
    bool isKingInCheck(Colour colour) const;
    bool isInsufficientMaterial() const;
    int countPieces() const;
    const Piece& getPiece(int square) const;
    const Piece& getPiece(int rank, int file) const;
    void setPiece(int square, Piece piece);
    void setPiece(int rank, int file, Piece piece);
    Colour getTurn() const;
    void setTurn(Colour colour);
    CastleRights getCastleRights(Colour colour) const;
    void setCastleRights(Colour colour, CastleRights rights);
    int getEnPassantSquare() const;
    void setEnPassantSquare(int square);
    const std::array<Piece, 64>& getSquares() const;

    std::uint64_t getZobristKey() const;

    void clear();
};