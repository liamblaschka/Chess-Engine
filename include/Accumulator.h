#pragma once

#include <array>
#include <vector>
#include <fstream>
#include <cstdint>

template <int INPUT_SIZE, int OUTPUT_SIZE>
class alignas(64) Accumulator {
private:
    std::vector<std::int16_t> weight;
    std::array<std::int16_t, OUTPUT_SIZE> bias;

    std::array<std::int16_t, OUTPUT_SIZE> values;

public:
    Accumulator();

    void refreshAccumulator(const std::vector<int>& active_features);

    void updateAccumulator(const std::vector<int>& added_features, const std::vector<int>& removed_features);

    const std::array<std::int16_t, OUTPUT_SIZE>& getValues() const;

    void load_weights(std::ifstream& file);
};