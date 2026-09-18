#pragma once

#include "LinearLayer.h"
#include "Accumulator.h"
#include <vector>
#include <array>
#include <string>
#include <cstdint>

class NNUE {
private:
    static constexpr int FEATURE_SIZE = 40960;
    static constexpr int A_SIZE = 256;
    static constexpr int H1_SIZE = 32;
    static constexpr int H2_SIZE = 16;

    Accumulator accumulator_w;
    Accumulator accumulator_b;
    LinearLayer h1;
    LinearLayer h2;
    LinearLayer output;
public:
    NNUE(const std::string& weights_file);

    float forward(int side_to_move);

    void refreshWhiteAccumulator(const std::vector<int>& active_features);
    void refreshBlackAccumulator(const std::vector<int>& active_features);

    void updateWhiteAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features);
    void updateBlackAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features);

    void crelu16(const std::int16_t* input, std::int8_t* output, int size);
    void crelu32(const std::int32_t* input, std::int8_t* output, int size);
};
