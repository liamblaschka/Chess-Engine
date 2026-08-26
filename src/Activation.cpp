#include "Activation.h"
#include <vector>
#include <algorithm>

void crelu(std::vector<float>& input) {
    for (float& value : input) {
        value = std::clamp(value, 0.0f, 1.0f);
    }
}