#pragma once

#include "Move.h"
#include "Board.h"
#include <vector>

class MoveGenerator {
private:
    std::vector<Move> legal_moves;
    std::vector<Move> pseudo_legal_moves;

    bool en_passant_possible;

    void generatePawnMoves(const Board& board, Colour turn, int rank, int file);
    void generateKnightMoves(const Board& board, Colour turn, int rank, int file);
    void generateSlidingMoves(const Board& board, Colour turn, int rank, int file, const int directions[][2], int direction_count);
    void generateBishopMoves(const Board& board, Colour turn, int rank, int file);
    void generateRookMoves(const Board& board, Colour turn, int rank, int file);
    void generateQueenMoves(const Board& board, Colour turn, int rank, int file);
    void generateKingMoves(const Board& board, Colour turn, int rank, int file);
public:
    MoveGenerator();
    std::vector<Move> generatePseudoLegalMoves(const Board& board);
    void generateLegalMoves(Board& board);

    const std::vector<Move>& getLegalMoves() const;
    bool isEnPassantPossible() const;
};