#pragma once

#include <vector>
#include <fstream>
#include <cstdint>

class Accumulator {
private:
    int input_size;
    int output_size;

    std::vector<std::int16_t> weight;
    std::vector<std::int16_t> bias;
public:
    Accumulator(int input_size, int output_size);

    std::vector<std::int16_t> refresh_accumulator(const std::vector<int>& active_features) const;

    void update_accumulator(std::vector<std::int16_t>& values, const std::vector<int>& added_features, const std::vector<int>& removed_features) const;

    void load_weights(std::ifstream& file);
};