#include "TranspositionTable.h"
#include <algorithm>
#include <cstring>

TranspositionTable::TranspositionTable(std::size_t size_mb) {
    const std::size_t bytes = size_mb * 1024 * 1024;

    num_entries = bytes / sizeof(AtomicTTEntry);

    if (num_entries == 0) {
        num_entries = 1;
    }

    table = std::make_unique<AtomicTTEntry[]>(num_entries);

    clear();
}

std::uint64_t TranspositionTable::packData(TTFlag flag, float value, int depth) {
    std::uint32_t value_bits;
    static_assert(sizeof(value_bits) == sizeof(value));

    std::memcpy(&value_bits, &value, sizeof(value_bits));

    const std::uint16_t packed_depth = static_cast<std::uint16_t>(std::clamp(depth, 0, 65535));

    /*
        bits  0 - 31 : float value
        bits 32 - 47 : depth
        bits 48 - 55 : flag
        bits 56 - 63 : unused
    */

    return static_cast<std::uint64_t>(value_bits) |
           (static_cast<std::uint64_t>(packed_depth) << 32) |
           (static_cast<std::uint64_t>(flag) << 48);
}

TTEntry TranspositionTable::unpackData(std::uint64_t data) {
    const std::uint32_t value_bits = static_cast<std::uint32_t>(data & 0xFFFFFFFFULL);

    float value;
    std::memcpy(&value, &value_bits, sizeof(value));

    const int depth = static_cast<int>((data >> 32) & 0xFFFFULL);
    const TTFlag flag = static_cast<TTFlag>((data >> 48) & 0xFFULL);

    TTEntry entry;
    entry.flag = flag;
    entry.value = value;
    entry.depth = depth;

    return entry;
}

std::uint32_t TranspositionTable::packMove(const Move& move) {
    /*
        bits  0 -  5 : from
        bits  6 - 11 : to
        bits 12 - 13 : MoveType
        bits 14 - 16 : promotion PieceType
        bits 17 - 18 : promotion Colour
    */

    return static_cast<std::uint32_t>(move.from) |
           (static_cast<std::uint32_t>(move.to) << 6) |
           (static_cast<std::uint32_t>(move.type) << 12) |
           (static_cast<std::uint32_t>(move.promotion_piece.type) << 14) |
           (static_cast<std::uint32_t>(move.promotion_piece.colour) << 17);
}

Move TranspositionTable::unpackMove(std::uint32_t data) {
    const int from = static_cast<int>(data & 0x3F);
    const int to = static_cast<int>((data >> 6) & 0x3F);

    const MoveType type = static_cast<MoveType>((data >> 12) & 0x3);

    const PieceType promotion_type = static_cast<PieceType>((data >> 14) & 0x7);

    const Colour promotion_colour = static_cast<Colour>((data >> 17) & 0x3);

    return Move(from, to, type, Piece(promotion_type, promotion_colour));
}

bool TranspositionTable::probe(std::uint64_t key, TTEntry& result) const {
    if (key == 0) {
        return false;
    }

    const std::size_t index = key % num_entries;
    const AtomicTTEntry& entry = table[index];

    const std::uint64_t key_before = entry.key.load(std::memory_order_acquire);

    if (key_before != key) {
        return false;
    }

    const std::uint64_t data = entry.data.load(std::memory_order_relaxed);
    const std::uint32_t move = entry.move.load(std::memory_order_relaxed);

    const std::uint64_t key_after = entry.key.load(std::memory_order_acquire);

    if (key_before != key_after || key_after != key) {
        return false;
    }

    result = unpackData(data);
    result.best_move = unpackMove(move);

    return true;
}

void TranspositionTable::store(std::uint64_t key, TTFlag flag, float value, int depth, const Move& best_move) {
    if (key == 0) {
        return;
    }

    const std::size_t index = key % num_entries;
    AtomicTTEntry& entry = table[index];

    const std::uint64_t old_key = entry.key.load(std::memory_order_relaxed);
    const std::uint64_t old_data = entry.data.load(std::memory_order_relaxed);

    bool replace = false;

    if (old_key == 0) {
        replace = true;
    } else if (old_key != key) {
        replace = true;
    } else {
        const TTEntry old_entry = unpackData(old_data);

        if (depth >= old_entry.depth) {
            replace = true;
        }
    }

    if (!replace) {
        return;
    }

    const std::uint64_t new_data = packData(flag, value, depth);
    const std::uint32_t new_move = packMove(best_move);

    /*
        Publish the move and data before publishing the key.
    */
    entry.move.store(new_move, std::memory_order_relaxed);
    entry.data.store(new_data, std::memory_order_relaxed);
    entry.key.store(key, std::memory_order_release);
}

void TranspositionTable::clear() {
    for (std::size_t i = 0; i < num_entries; ++i) {
        table[i].move.store(0, std::memory_order_relaxed);
        table[i].data.store(0, std::memory_order_relaxed);
        table[i].key.store(0, std::memory_order_relaxed);
    }
}

std::size_t TranspositionTable::size() const {
    return num_entries;
}