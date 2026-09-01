#pragma once

#include <vector>
#include <fstream>

class LinearLayer {
private:
    int input_size;
    int output_size;

    std::vector<float> weight;
    std::vector<float> bias;
public:
    LinearLayer(int input_size, int output_size);

    std::vector<float> forward(const std::vector<float>& input) const;

    void load_weights(std::ifstream& file);
};