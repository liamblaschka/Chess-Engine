#pragma once

#include "Move.h"
#include <cstdint>
#include <vector>

enum class TTFlag : std::uint8_t {
    Exact,
    LowerBound,
    UpperBound
};

struct TTEntry {
    std::uint64_t key = 0;

    TTFlag flag = TTFlag::Exact;

    float value = 0.0f;
    int depth = -1;
};

class TranspositionTable {
private:
    std::vector<TTEntry> table;

public:
    explicit TranspositionTable(std::size_t size_mb = 64);

    TTEntry* probe(std::uint64_t key);

    void store(std::uint64_t key, TTFlag flag, float value, int depth);

    void clear();

    std::size_t size() const;
};