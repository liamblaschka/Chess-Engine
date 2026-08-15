#pragma once

#include "Move.h"
#include "Board.h"
#include <vector>

class MoveGenerator {
private:
    void generatePawnMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateKnightMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateBishopMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateRookMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
public:
    MoveGenerator();
    std::vector<Move> generatePseudoLegalMoves(const Board& board);
};