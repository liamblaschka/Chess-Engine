#include "NNUE.h"
#include "NNUEModel.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstdint>

float NNUE::forward(int side_to_move) const {
    return model.forward(w_acc_values, b_acc_values, side_to_move);
}

void NNUE::refreshWhiteAccumulator(const std::vector<int>& active_features) {
    model.refreshWhiteAccumulator(w_acc_values.data(), active_features);
}

void NNUE::refreshBlackAccumulator(const std::vector<int>& active_features) {
    model.refreshBlackAccumulator(b_acc_values.data(), active_features);
}

void NNUE::updateWhiteAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features) {
    model.updateWhiteAccumulator(w_acc_values.data(), added_features, removed_features);
}

void NNUE::updateBlackAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features) {
    model.updateBlackAccumulator(b_acc_values.data(), added_features, removed_features);
}

void NNUE::loadModel(const std::string& weights_file) {
    model.load(weights_file);
}