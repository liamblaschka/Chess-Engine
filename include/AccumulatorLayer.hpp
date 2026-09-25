#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <cstdint>

template <int INPUT_SIZE, int OUTPUT_SIZE>
class AccumulatorLayer {
private:
    std::vector<std::int16_t> weight;
    alignas(64) std::array<std::int16_t, OUTPUT_SIZE> bias;

public:
    AccumulatorLayer();

    void refreshAccumulator(std::int16_t* values, const std::vector<int>& active_features) const;

    void updateAccumulator(std::int16_t* values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const;

    void load_weights(std::ifstream& file);
};


template <int INPUT_SIZE, int OUTPUT_SIZE>
AccumulatorLayer<INPUT_SIZE, OUTPUT_SIZE>::AccumulatorLayer() : weight(INPUT_SIZE * OUTPUT_SIZE) {}

template <int INPUT_SIZE, int OUTPUT_SIZE>
void AccumulatorLayer<INPUT_SIZE, OUTPUT_SIZE>::refreshAccumulator(std::int16_t* values, const std::vector<int>& active_features) const {
    values = static_cast<std::int16_t*>(__builtin_assume_aligned(values, 64));
    
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
void AccumulatorLayer<INPUT_SIZE, OUTPUT_SIZE>::updateAccumulator(std::int16_t* values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    values = static_cast<std::int16_t*>(__builtin_assume_aligned(values, 64));
    
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
void AccumulatorLayer<INPUT_SIZE, OUTPUT_SIZE>::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(std::int16_t));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(std::int16_t));
}