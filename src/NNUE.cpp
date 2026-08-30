#include "NNUE.h"
#include "LinearLayer.h"
#include "Activation.h"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

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

float NNUE::forward(const std::vector<float>& white_acc_values, const std::vector<float>& black_acc_values, int side_to_move) {
    std::vector<float> acc_values;
    acc_values.reserve(white_acc_values.size() + black_acc_values.size());
    if (side_to_move == 0) {
        acc_values.insert(acc_values.end(), white_acc_values.begin(), white_acc_values.end());
        acc_values.insert(acc_values.end(), black_acc_values.begin(), black_acc_values.end());
    } else {
        acc_values.insert(acc_values.end(), black_acc_values.begin(), black_acc_values.end());
        acc_values.insert(acc_values.end(), white_acc_values.begin(), white_acc_values.end());
    }
    crelu(acc_values);

    std::vector<float> h1_output = h1.forward(acc_values);
    crelu(h1_output);

    std::vector<float> h2_output = h2.forward(h1_output);
    crelu(h2_output);

    return output.forward(h2_output)[0];
}

void NNUE::refreshWhiteAccumulator(std::vector<float>& acc_values, const std::vector<int>& active_features) const {
    acc_values = accumulator_w.refresh_accumulator(active_features);
}

void NNUE::refreshBlackAccumulator(std::vector<float>& acc_values, const std::vector<int>& active_features) const {
    acc_values = accumulator_b.refresh_accumulator(active_features);
}

void NNUE::updateWhiteAccumulator(std::vector<float>& acc_values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    accumulator_w.update_accumulator(acc_values, added_features, removed_features);
}

void NNUE::updateBlackAccumulator(std::vector<float>& acc_values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const {
    accumulator_b.update_accumulator(acc_values, added_features, removed_features);
}
