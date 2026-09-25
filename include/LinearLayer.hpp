#pragma once

#include <array>
#include <fstream>
#include <cstdint>

template <int INPUT_SIZE, int OUTPUT_SIZE>
class LinearLayer {
private:
    alignas(64) std::array<std::int8_t, INPUT_SIZE * OUTPUT_SIZE> weight;
    alignas(64) std::array<std::int32_t, OUTPUT_SIZE> bias;
public:
    void forward(const std::int8_t* input, int32_t* output) const;

    void load_weights(std::ifstream& file);
};


template <int INPUT_SIZE, int OUTPUT_SIZE>
void LinearLayer<INPUT_SIZE, OUTPUT_SIZE>::forward(const std::int8_t* __restrict__ input, int32_t* __restrict__ output) const {
    input = static_cast<const std::int8_t*>(__builtin_assume_aligned(input, 64));
    output = static_cast<std::int32_t*>(__builtin_assume_aligned(output, 64));

    for (int i = 0; i < OUTPUT_SIZE; i++) {
        output[i] = bias[i];
    }

    for (int i = 0; i < INPUT_SIZE; i++) {
        for (int j = 0; j < OUTPUT_SIZE; j++) {
            output[j] += input[i] * weight[i * OUTPUT_SIZE + j];
        }
    }

    for (int i = 0; i < OUTPUT_SIZE; i++) {
        output[i] /= 64;
    }
}

template <int INPUT_SIZE, int OUTPUT_SIZE>
void LinearLayer<INPUT_SIZE, OUTPUT_SIZE>::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(std::int8_t));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(std::int32_t));
}