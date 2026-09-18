#include "Accumulator.h"
#include <array>
#include <vector>
#include <fstream>
#include <cstdint>

Accumulator::Accumulator() : weight(INPUT_SIZE * OUTPUT_SIZE) {}

void Accumulator::refreshAccumulator(const std::vector<int>& active_features) {
    for (int i = 0; i < OUTPUT_SIZE; i++) {
        values[i] = bias[i];
    }

    for (int feature : active_features) {
        for (int i = 0; i < OUTPUT_SIZE; i++) {
            values[i] += weight[feature * OUTPUT_SIZE + i];
        }
    }
}

void Accumulator::updateAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features) {
    for (int feature : removed_features) {
        for (int i = 0; i < OUTPUT_SIZE; i++) {
            values[i] -= weight[feature * OUTPUT_SIZE + i];
        }
    }

    for (int feature : added_features) {
        for (int i = 0; i < OUTPUT_SIZE; i++) {
            values[i] += weight[feature * OUTPUT_SIZE + i];
        }
    }
}

const std::array<std::int16_t, Accumulator::OUTPUT_SIZE>& Accumulator::getValues() const { return values; }

void Accumulator::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(std::int16_t));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(std::int16_t));
}