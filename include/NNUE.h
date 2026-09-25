#pragma once

#include "NNUEModel.hpp"
#include <vector>
#include <array>
#include <string>
#include <cstdint>

class NNUE {
private:
    static constexpr int FEATURE_SIZE = 40960;
    static constexpr int A_SIZE = 512;
    static constexpr int H1_SIZE = 32;
    static constexpr int H2_SIZE = 32;

    inline static NNUEModel<FEATURE_SIZE, A_SIZE, H1_SIZE, H2_SIZE> model;

    alignas(64) std::array<std::int16_t, A_SIZE> w_acc_values;
    alignas(64) std::array<std::int16_t, A_SIZE> b_acc_values;

public:
    float forward(int side_to_move) const;

    void refreshWhiteAccumulator(const std::vector<int>& active_features);
    void refreshBlackAccumulator(const std::vector<int>& active_features);

    void updateWhiteAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features);
    void updateBlackAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features);

    static void loadModel(const std::string& weights_file);
};