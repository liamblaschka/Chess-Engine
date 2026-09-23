#pragma once

#include "Move.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>

enum class TTFlag : std::uint8_t {
    Exact,
    LowerBound,
    UpperBound
};

struct TTEntry {
    TTFlag flag = TTFlag::Exact;
    float value = 0.0f;
    int depth = -1;
    Move best_move;
};

class TranspositionTable {
private:
    struct AtomicTTEntry {
        std::atomic<std::uint64_t> key{0};
        std::atomic<std::uint64_t> data{0};
        std::atomic<std::uint32_t> move{0};
    };

    std::unique_ptr<AtomicTTEntry[]> table;
    std::size_t num_entries;

    static std::uint64_t packData(TTFlag flag, float value, int depth);
    static TTEntry unpackData(std::uint64_t data);

    static std::uint32_t packMove(const Move& move);
    static Move unpackMove(std::uint32_t data);

public:
    explicit TranspositionTable(std::size_t size_mb = 64);

    bool probe(std::uint64_t key, TTEntry& result) const;

    void store(std::uint64_t key, TTFlag flag, float value, int depth, const Move& best_move);

    void clear();

    std::size_t size() const;
};