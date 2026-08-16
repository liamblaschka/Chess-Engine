#pragma once

#include "Move.h"
#include "Board.h"
#include <vector>

class MoveGenerator {
private:
    void generatePawnMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateKnightMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateSlidingMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file, const int directions[][2], int direction_count);
    void generateBishopMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateRookMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateQueenMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
    void generateKingMoves(const Board& board, std::vector<Move>& moves, Colour turn, int rank, int file);
public:
    MoveGenerator();
    std::vector<Move> generatePseudoLegalMoves(const Board& board);
    std::vector<Move> generateLegalMoves(Board& board);
};