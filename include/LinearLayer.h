#pragma once

#include <array>
#include <fstream>
#include <cstdint>

template <int INPUT_SIZE, int OUTPUT_SIZE>
class LinearLayer {
private:
    int input_size;
    int output_size;

    std::array<std::int8_t, INPUT_SIZE * OUTPUT_SIZE> weight;
    std::array<std::int32_t, OUTPUT_SIZE> bias;
public:
    void forward(const std::int8_t* input, int32_t* output) const;

    void load_weights(std::ifstream& file);
};