#pragma once

#include "Move.h"
#include "Board.h"
#include <vector>

class MoveGenerator {
private:
    void generatePawnMoves(const Board& board, std::vector<Move>& moves, int rank, int file);
public:
    MoveGenerator();
    std::vector<Move> generatePseudoLegalMoves(const Board& board);
};