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
    // Preserve the exact 32-bit representation of the float.
    std::uint32_t value_bits;
    static_assert(sizeof(value_bits) == sizeof(value));

    std::memcpy(&value_bits, &value, sizeof(value_bits));

    // depth = -1 is reserved for an empty/invalid entry.
    const std::uint16_t packed_depth = static_cast<std::uint16_t>(std::clamp(depth, 0, 65535));

    /*
        Layout:

        bits  0 - 31 : float value
        bits 32 - 47 : depth
        bits 48 - 55 : flag
        bits 56 - 63 : unused
    */

    return
        static_cast<std::uint64_t>(value_bits) |
        (static_cast<std::uint64_t>(packed_depth) << 32) |
        (static_cast<std::uint64_t>(flag) << 48);
}

TTEntry TranspositionTable::unpackData(std::uint64_t data) {
    const std::uint32_t value_bits =
        static_cast<std::uint32_t>(data & 0xFFFFFFFFULL);

    float value;

    std::memcpy(&value, &value_bits, sizeof(value));

    const int depth = static_cast<int>((data >> 32) & 0xFFFFULL);

    const TTFlag flag = static_cast<TTFlag>((data >> 48) & 0xFFULL);

    return {flag, value, depth};
}

bool TranspositionTable::probe(std::uint64_t key, TTEntry& result) const {
    // key == 0 is used to represent an empty entry.
    if (key == 0) {
        return false;
    }

    const std::size_t index = key % num_entries;

    const AtomicTTEntry& entry = table[index];

    /*
        Read the key before and after reading data.

        If another thread replaces this slot while we're reading it,
        the two key reads should differ and the result is discarded.
    */

    const std::uint64_t key_before = entry.key.load(std::memory_order_acquire);

    if (key_before != key) {
        return false;
    }

    const std::uint64_t data = entry.data.load(std::memory_order_relaxed);

    const std::uint64_t key_after = entry.key.load(std::memory_order_acquire);

    if (key_before != key_after || key_after != key) {
        return false;
    }

    result = unpackData(data);

    return true;
}

void TranspositionTable::store(std::uint64_t key, TTFlag flag, float value, int depth) {
    // Reserve key 0 for empty entries.
    if (key == 0) {
        return;
    }

    const std::size_t index = key % num_entries;

    AtomicTTEntry& entry = table[index];

    const std::uint64_t old_key = entry.key.load(std::memory_order_relaxed);

    const std::uint64_t old_data = entry.data.load(std::memory_order_relaxed);

    bool replace = false;

    if (old_key == 0) {
        // Empty entry.
        replace = true;
    }
    else if (old_key != key) {
        // Collision: allow replacement.
        replace = true;
    }
    else {
        // Same position: keep the deeper search.
        const TTEntry old_entry = unpackData(old_data);

        if (depth >= old_entry.depth) {
            replace = true;
        }
    }

    if (!replace) {
        return;
    }

    const std::uint64_t new_data = packData(flag, value, depth);

    /*
        Publish data first, then publish its corresponding key.

        release on the key store ensures the preceding data store
        becomes visible before a reader observes this key.
    */
    entry.data.store(new_data, std::memory_order_relaxed);

    entry.key.store(key, std::memory_order_release);
}

void TranspositionTable::clear() {
    for (std::size_t i = 0; i < num_entries; ++i) {
        table[i].data.store(0, std::memory_order_relaxed);

        table[i].key.store(0, std::memory_order_relaxed);
    }
}

std::size_t TranspositionTable::size() const {
    return num_entries;
}