#pragma once

#include "LinearLayer.h"
#include "Accumulator.h"
#include <vector>
#include <string>

class NNUE {
private:
    static constexpr int FEATURE_SIZE = 768;
    static constexpr int A_SIZE = 128;
    static constexpr int H1_SIZE = 16;

    Accumulator accumulator;
    LinearLayer h1;
    LinearLayer output;
public:
    NNUE(const std::string& weights_file);

    float forward(const std::vector<int>& active_features);
};
