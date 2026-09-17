#pragma once

#include <vector>
#include <fstream>
#include <cstdint>

class LinearLayer {
private:
    int input_size;
    int output_size;

    std::vector<std::int8_t> weight;
    std::vector<std::int32_t> bias;
public:
    LinearLayer(int input_size, int output_size);

    std::vector<std::int32_t> forward(const std::vector<std::int8_t>& input) const;

    void load_weights(std::ifstream& file);
};