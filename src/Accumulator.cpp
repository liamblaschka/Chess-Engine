#include "Accumulator.h"
#include <vector>
#include <fstream>

Accumulator::Accumulator(int input_size, int output_size)
    : input_size(input_size), output_size(output_size), weight(input_size * output_size), bias(output_size) {}

std::vector<float> Accumulator::refresh_accumulator(const std::vector<int>& active_features) const {
    std::vector<float> output(output_size);

    for (int i = 0; i < output_size; i++) {
        output[i] = bias[i];
    }

    for (int feature : active_features) {
        for (int i = 0; i < output_size; i++) {
            output[i] += weight[feature * output_size + i];
        }
    }

    return output;
}

void Accumulator::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(float));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(float));
}