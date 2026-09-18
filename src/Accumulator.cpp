#include "Accumulator.h"
#include <array>
#include <vector>
#include <fstream>
#include <cstdint>

template <int INPUT_SIZE, int OUTPUT_SIZE>
Accumulator<INPUT_SIZE, OUTPUT_SIZE>::Accumulator() : weight(INPUT_SIZE * OUTPUT_SIZE) {}

template <int INPUT_SIZE, int OUTPUT_SIZE>
void Accumulator<INPUT_SIZE, OUTPUT_SIZE>::refreshAccumulator(const std::vector<int>& active_features) {
    for (int i = 0; i < OUTPUT_SIZE; i++) {
        values[i] = bias[i];
    }

    for (int feature : active_features) {
        for (int i = 0; i < OUTPUT_SIZE; i++) {
            values[i] += weight[feature * OUTPUT_SIZE + i];
        }
    }
}

template <int INPUT_SIZE, int OUTPUT_SIZE>
void Accumulator<INPUT_SIZE, OUTPUT_SIZE>::updateAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features) {
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

template <int INPUT_SIZE, int OUTPUT_SIZE>
const std::array<std::int16_t, OUTPUT_SIZE>& Accumulator<INPUT_SIZE, OUTPUT_SIZE>::getValues() const { return values; }

template <int INPUT_SIZE, int OUTPUT_SIZE>
void Accumulator<INPUT_SIZE, OUTPUT_SIZE>::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(std::int16_t));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(std::int16_t));
}