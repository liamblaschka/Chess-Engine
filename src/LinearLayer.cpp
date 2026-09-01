#include "LinearLayer.h"
#include <vector>
#include <fstream>

LinearLayer::LinearLayer(int input_size, int output_size)
    : input_size(input_size), output_size(output_size), weight(input_size * output_size), bias(output_size) {}

std::vector<float> LinearLayer::forward(const std::vector<float>& input) const {
    std::vector<float> output(output_size);

    for (int i = 0; i < output_size; i++) {
        output[i] = bias[i];
    }

    for (int i = 0; i < input_size; i++) {
        for (int j = 0; j < output_size; j++) {
            output[j] += input[i] * weight[i * output_size + j];
        }
    }

    return output;
}

void LinearLayer::load_weights(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(weight.data()), weight.size() * sizeof(float));
    file.read(reinterpret_cast<char*>(bias.data()), bias.size() * sizeof(float));
}