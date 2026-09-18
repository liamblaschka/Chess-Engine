#pragma once

#include "Accumulator.hpp"
#include "LinearLayer.hpp"
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

    Accumulator<FEATURE_SIZE, A_SIZE> accumulator_w;
    Accumulator<FEATURE_SIZE, A_SIZE> accumulator_b;
    LinearLayer<A_SIZE * 2, H1_SIZE> h1;
    LinearLayer<H1_SIZE, H2_SIZE> h2;
    LinearLayer<H2_SIZE, 1> output;
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
