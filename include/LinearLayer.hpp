#pragma once

#include <array>
#include <fstream>
#include <cstdint>

template <int INPUT_SIZE, int OUTPUT_SIZE>
class LinearLayer {
private:
    std::array<std::int8_t, INPUT_SIZE * OUTPUT_SIZE> weight;
    std::array<std::int32_t, OUTPUT_SIZE> bias;
public:
    void forward(const std::int8_t* input, int32_t* output) const;

    void load_weights(std::ifstream& file);
};


template <int INPUT_SIZE, int OUTPUT_SIZE>
void LinearLayer<INPUT_SIZE, OUTPUT_SIZE>::forward(const std::int8_t* input, int32_t* output) const {
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

// std::vector<std::int32_t> LinearLayer::forward(const std::vector<std::int8_t>& input) const {
//     std::vector<std::int32_t> output(output_size);

//     for (int i = 0; i < output_size; i++) {
//         output[i] = bias[i];
//     }

//     for (int i = 0; i < input_size; i++) {
//         for (int j = 0; j < output_size; j++) {
//             output[j] += input[i] * weight[i * output_size + j];
//         }
//     }

//     for (auto& value : output) {
//         value /= 64;
//     }

//     return output;
// }

template <int INPUT_SIZE, int OUTPUT_SIZE>
void LinearLayer<INPUT_SIZE, OUTPUT_SIZE>::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(std::int8_t));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(std::int32_t));
}