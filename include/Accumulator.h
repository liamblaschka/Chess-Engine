#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <cstdint>

class alignas(64) Accumulator {
private:
    static constexpr int INPUT_SIZE = 40960;
    static constexpr int OUTPUT_SIZE = 256;

    std::array<std::int16_t, INPUT_SIZE * OUTPUT_SIZE> weight;
    std::array<std::int16_t, OUTPUT_SIZE> bias;

    std::array<std::int16_t, OUTPUT_SIZE> values;

public:
    void refreshAccumulator(const std::vector<int>& active_features);

    void updateAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features);

    std::array<std::int16_t, OUTPUT_SIZE>& getValues();

    void load_weights(std::ifstream& file);
};