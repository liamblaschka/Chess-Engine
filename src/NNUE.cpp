#include "NNUE.h"
#include "LinearLayer.h"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

NNUE::NNUE(const std::string& weights_file)
    : accumulator_w(FEATURE_SIZE, A_SIZE), accumulator_b(FEATURE_SIZE, A_SIZE), h1(A_SIZE * 2, H1_SIZE), h2(H1_SIZE, H2_SIZE), output(H2_SIZE, 1)
{
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

float NNUE::forward(const std::vector<std::int16_t>& white_acc_values, const std::vector<std::int16_t>& black_acc_values, int side_to_move) {
    std::vector<std::int16_t> acc_values;
    acc_values.reserve(white_acc_values.size() + black_acc_values.size());
    if (side_to_move == 0) {
        acc_values.insert(acc_values.end(), white_acc_values.begin(), white_acc_values.end());
        acc_values.insert(acc_values.end(), black_acc_values.begin(), black_acc_values.end());
    } else {
        acc_values.insert(acc_values.end(), black_acc_values.begin(), black_acc_values.end());
        acc_values.insert(acc_values.end(), white_acc_values.begin(), white_acc_values.end());
    }
    std::vector<std::int8_t> acc_activated = crelu16(acc_values);

    std::vector<std::int32_t> h1_output = h1.forward(acc_activated);
    std::vector<std::int8_t> h1_activated = crelu32(h1_output);

    std::vector<std::int32_t> h2_output = h2.forward(h1_activated);
    std::vector<std::int8_t> h2_activated = crelu32(h2_output);

    return output.forward(h2_activated)[0];
}

void NNUE::refreshWhiteAccumulator(std::vector<std::int16_t>& acc_values, const std::vector<int>& active_features) const {
    acc_values = accumulator_w.refresh_accumulator(active_features);
}

void NNUE::refreshBlackAccumulator(std::vector<std::int16_t>& acc_values, const std::vector<int>& active_features) const {
    acc_values = accumulator_b.refresh_accumulator(active_features);
}

void NNUE::updateWhiteAccumulator(std::vector<std::int16_t>& acc_values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    accumulator_w.update_accumulator(acc_values, added_features, removed_features);
}

void NNUE::updateBlackAccumulator(std::vector<std::int16_t>& acc_values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    accumulator_b.update_accumulator(acc_values, added_features, removed_features);
}

std::vector<std::int8_t> NNUE::crelu16(const std::vector<std::int16_t>& input) {
    std::vector<std::int8_t> output;
    output.reserve(input.size());
    for (std::int16_t value : input) {
        output.push_back(static_cast<std::int8_t>(std::clamp<std::int16_t>(value, 0, 127)));
    }

    return output;
}

std::vector<std::int8_t> NNUE::crelu32(const std::vector<std::int32_t>& input) {
    std::vector<std::int8_t> output;
    output.reserve(input.size());
    for (std::int32_t value : input) {
        output.push_back(static_cast<std::int8_t>(std::clamp<std::int32_t>(value, 0, 127)));
    }

    return output;
}