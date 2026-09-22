#pragma once

#include <cstdint>
#include <random>

namespace Zobrist {
    inline std::uint64_t piece[2][6][64];
    inline std::uint64_t side_to_move;
    inline std::uint64_t castle_rights[4];
    inline std::uint64_t en_passant[8];

    inline void initialise() {
        std::mt19937_64 rng(33);

        for (int colour = 0; colour < 2; colour++) {
            for (int type = 0; type < 6; type++) {
                for (int square = 0; square < 64; square++) {
                    piece[colour][type][square] = rng();
                }
            }
        }

        side_to_move = rng();

        for (int i = 0; i < 4; i++) {
            castle_rights[i] = rng();
        }

        for (int i = 0; i < 8; i++) {
            en_passant[i] = rng();
        }
    }
}