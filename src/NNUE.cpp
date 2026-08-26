#include "NNUE.h"
#include "LinearLayer.h"
#include "Activation.h"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

NNUE::NNUE(const std::string& weights_file) : accumulator(FEATURE_SIZE, A_SIZE), h1(A_SIZE, H1_SIZE), output(H1_SIZE, 1) {
    std::ifstream file(weights_file, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open NNUE weights file.");
    }

    accumulator.load_weights(file);
    h1.load_weights(file);
    output.load_weights(file);
}

float NNUE::forward(const std::vector<int>& active_features) {
    std::vector<float> a_output = accumulator.refresh_accumulator(active_features);
    crelu(a_output);

    std::vector<float> h1_output = h1.forward(a_output);
    crelu(h1_output);

    return output.forward(h1_output)[0];
}
