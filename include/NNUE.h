#pragma once

#include "LinearLayer.h"
#include "Accumulator.h"
#include <vector>
#include <string>

class NNUE {
private:
    static constexpr int FEATURE_SIZE = 40960;
    static constexpr int A_SIZE = 256;
    static constexpr int H1_SIZE = 8;
    static constexpr int H2_SIZE = 16;

    Accumulator accumulator_w;
    Accumulator accumulator_b;
    LinearLayer h1;
    LinearLayer h2;
    LinearLayer output;
public:
    NNUE(const std::string& weights_file);

    float forward(const std::vector<float>& white_acc_values, const std::vector<float>& black_acc_values, int side_to_move);

    void refreshWhiteAccumulator(std::vector<float>& acc_values, const std::vector<int>& active_features) const;
    void refreshBlackAccumulator(std::vector<float>& acc_values, const std::vector<int>& active_features) const;

    void updateWhiteAccumulator(std::vector<float>& acc_values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const;
    void updateBlackAccumulator(std::vector<float>& acc_values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const;
};
