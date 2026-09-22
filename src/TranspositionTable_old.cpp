#include "TranspositionTable.h"
#include "Move.h"
#include <cstdint>
#include <vector>

TranspositionTable::TranspositionTable(std::size_t size_mb) {
    std::size_t bytes = size_mb * 1024 * 1024;
    std::size_t num_entries = bytes / sizeof(TTEntry);

    table.resize(num_entries);
}

TTEntry* TranspositionTable::probe(std::uint64_t key) {
    TTEntry& entry = table[key % table.size()];

    if (entry.key == key) {
        return &entry;
    }

    return nullptr;
}

void TranspositionTable::store(std::uint64_t key, TTFlag flag, float value, int depth) {
    TTEntry& entry = table[key % table.size()];
    if (entry.key != key || entry.depth >= depth) {
        entry.key = key;
        entry.flag = flag;
        // entry.best_move = best_move;
        entry.value = value;
        entry.depth = depth;
    }
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry{});
}

std::size_t TranspositionTable::size() const { return table.size(); }