#pragma once

#include "AccumulatorLayer.hpp"
#include "LinearLayer.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
class NNUEModel {
private:
    AccumulatorLayer<FEATURE_SIZE, A_SIZE> accumulator_w;
    AccumulatorLayer<FEATURE_SIZE, A_SIZE> accumulator_b;
    LinearLayer<A_SIZE * 2, H1_SIZE> h1;
    LinearLayer<H1_SIZE, H2_SIZE> h2;
    LinearLayer<H2_SIZE, 1> output;

    void crelu16(const std::int16_t* __restrict__ input, std::int8_t* __restrict__ output, int size) const;
    void crelu32(const std::int32_t* __restrict__ input, std::int8_t* __restrict__ output, int size) const;

public:
    float forward(const std::array<std::int16_t, A_SIZE>& acc_w_values, const std::array<std::int16_t, A_SIZE>& acc_b_values, int side_to_move) const;

    void refreshWhiteAccumulator(std::int16_t* values, const std::vector<int>& active_features) const;
    void refreshBlackAccumulator(std::int16_t* values, const std::vector<int>& active_features) const;

    void updateWhiteAccumulator(std::int16_t* values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const;
    void updateBlackAccumulator(std::int16_t* values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const;

    void load(const std::string& weights_file);
};


template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
float NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::forward(const std::array<std::int16_t, A_SIZE>& w_acc_values, const std::array<std::int16_t, A_SIZE>& b_acc_values, int side_to_move) const {
    alignas(64) std::array<std::int8_t, A_SIZE * 2> acc_activated;
    alignas(64) std::array<std::int32_t, H1_SIZE> h1_output;
    alignas(64) std::array<std::int8_t, H1_SIZE> h1_activated;
    alignas(64) std::array<std::int32_t, H2_SIZE> h2_output;
    alignas(64) std::array<std::int8_t, H2_SIZE> h2_activated;
    alignas(64) std::int32_t output_value;

    if (side_to_move == 0) {
        crelu16(w_acc_values.data(), acc_activated.data(), A_SIZE);
        crelu16(b_acc_values.data(), acc_activated.data() + A_SIZE, A_SIZE);
    } else {
        crelu16(b_acc_values.data(), acc_activated.data(), A_SIZE);
        crelu16(w_acc_values.data(), acc_activated.data() + A_SIZE, A_SIZE);
    }

    h1.forward(acc_activated.data(), h1_output.data());
    crelu32(h1_output.data(), h1_activated.data(), H1_SIZE);

    h2.forward(h1_activated.data(), h2_output.data());
    crelu32(h2_output.data(), h2_activated.data(), H2_SIZE);

    output.forward(h2_activated.data(), &output_value);

    return static_cast<float>(output_value) * 410 / 127;
}

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::refreshWhiteAccumulator(std::int16_t* values, const std::vector<int>& active_features) const {
    accumulator_w.refreshAccumulator(values, active_features);
}

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::refreshBlackAccumulator(std::int16_t* values, const std::vector<int>& active_features) const {
    accumulator_b.refreshAccumulator(values, active_features);
}

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::updateWhiteAccumulator(std::int16_t* values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    accumulator_w.updateAccumulator(values, added_features, removed_features);
}

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::updateBlackAccumulator(std::int16_t* values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    accumulator_b.updateAccumulator(values, added_features, removed_features);
}

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::crelu16(const std::int16_t* __restrict__ input, std::int8_t* __restrict__ output, int size) const {
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

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::crelu32(const std::int32_t* __restrict__ input, std::int8_t* __restrict__ output, int size) const {
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

template <int FEATURE_SIZE, int A_SIZE, int H1_SIZE, int H2_SIZE>
void NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE>::load(const std::string& weights_file) {
    std::ifstream file(weights_file, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open NNUEModel weights file.");
    }

    accumulator_w.load_weights(file);
    accumulator_b.load_weights(file);
    h1.load_weights(file);
    h2.load_weights(file);
    output.load_weights(file);
}