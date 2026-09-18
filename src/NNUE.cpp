#include "NNUE.h"
#include "Accumulator.hpp"
#include "LinearLayer.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

NNUE::NNUE(const std::string& weights_file) {
    std::ifstream file(weights_file, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open NNUE weights file.");
    }

    accumulator_w.load_weights(file);
    accumulator_b.load_weights(file);
    h1.load_weights(file);
    h2.load_weights(file);
    output.load_weights(file);
}

float NNUE::forward(int side_to_move) {
    alignas(64) std::array<std::int8_t, A_SIZE * 2> acc_activated;
    alignas(64) std::array<std::int32_t, H1_SIZE> h1_output;
    alignas(64) std::array<std::int8_t, H1_SIZE> h1_activated;
    alignas(64) std::array<std::int32_t, H2_SIZE> h2_output;
    alignas(64) std::array<std::int8_t, H2_SIZE> h2_activated;
    alignas(64) std::int32_t output_value;

    std::array<std::int16_t, A_SIZE> white_acc_values = accumulator_w.getValues();
    std::array<std::int16_t, A_SIZE> black_acc_values = accumulator_b.getValues();
    if (side_to_move == 0) {
        crelu16(white_acc_values.data(), acc_activated.data(), A_SIZE);
        crelu16(black_acc_values.data(), acc_activated.data() + A_SIZE, A_SIZE);
    } else {
        crelu16(black_acc_values.data(), acc_activated.data(), A_SIZE);
        crelu16(white_acc_values.data(), acc_activated.data() + A_SIZE, A_SIZE);
    }

    h1.forward(acc_activated.data(), h1_output.data());
    crelu32(h1_output.data(), h1_activated.data(), H1_SIZE);

    h2.forward(h1_activated.data(), h2_output.data());
    crelu32(h2_output.data(), h2_activated.data(), H2_SIZE);

    output.forward(h2_activated.data(), &output_value);

    return static_cast<float>(output_value) * 410 / 127;
}

void NNUE::refreshWhiteAccumulator(const std::vector<int>& active_features) {
    accumulator_w.refreshAccumulator(active_features);
}

void NNUE::refreshBlackAccumulator(const std::vector<int>& active_features) {
    accumulator_b.refreshAccumulator(active_features);
}

void NNUE::updateWhiteAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features) {
    accumulator_w.updateAccumulator(added_features, removed_features);
}

void NNUE::updateBlackAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features) {
    accumulator_b.updateAccumulator(added_features, removed_features);
}

void NNUE::crelu16(const std::int16_t* __restrict__ input, std::int8_t* __restrict__ output, int size) {
    input = static_cast<const std::int16_t*>(__builtin_assume_aligned(input, 64));
    output = static_cast<std::int8_t*>(__builtin_assume_aligned(output, 64));

    for (int i = 0; i < size; i++) {
        std::int16_t value = input[i];
        if (value < 0) {
            output[i] = 0;
        } else if (value > 127) {
            output[i] = 127;
        } else {
            output[i] = static_cast<std::int8_t>(value);
        }
    }
}

void NNUE::crelu32(const std::int32_t* __restrict__ input, std::int8_t* __restrict__ output, int size) {
    input = static_cast<const std::int32_t*>(__builtin_assume_aligned(input, 64));
    output = static_cast<std::int8_t*>(__builtin_assume_aligned(output, 64));

    for (int i = 0; i < size; i++) {
        std::int32_t value = input[i];
        if (value < 0) {
            output[i] = 0;
        } else if (value > 127) {
            output[i] = 127;
        } else {
            output[i] = static_cast<std::int8_t>(value);
        }
    }
}