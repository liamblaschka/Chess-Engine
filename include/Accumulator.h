#pragma once

#include <vector>
#include <fstream>

class Accumulator {
private:
    int input_size;
    int output_size;

    std::vector<float> weight;
    std::vector<float> bias;
public:
    Accumulator(int input_size, int output_size);

    std::vector<float> refresh_accumulator(const std::vector<int>& active_features) const;

    void load_weights(std::ifstream& file);
};