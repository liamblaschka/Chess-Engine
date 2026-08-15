#pragma once

#include "Piece.h"
#include <array>

class Board {
private:
    std::array<Piece, 64> squares;
public:
    Board();
    void draw();
};