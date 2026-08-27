#pragma once

#include "LinearLayer.h"
#include "Accumulator.h"
#include <vector>
#include <string>

class NNUE {
private:
    static constexpr int FEATURE_SIZE = 768;
    static constexpr int A_SIZE = 64;
    static constexpr int H1_SIZE = 32;

    Accumulator accumulator_w;
    Accumulator accumulator_b;
    LinearLayer h1;
    LinearLayer output;
public:
    NNUE(const std::string& weights_file);

    float forward(const std::vector<float>& white_acc_values, const std::vector<float>& black_acc_values, int side_to_move);

    void refreshAccumulators(
        std::vector<float>& white_values, std::vector<float>& black_values,
        const std::vector<int>& active_features
    ) const;

    void updateAccumulators(
        std::vector<float>& white_values, std::vector<float>& black_values,
        const std::vector<int>& added_features, const std::vector<int>& removed_features
    ) const;
};
