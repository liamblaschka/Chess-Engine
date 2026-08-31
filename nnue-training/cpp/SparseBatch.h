#pragma once

#include "Dataset.h"
#include <memory>

class SparseBatch {
private:
    int size;
    
    static constexpr int MAX_ACTIVE_FEATURES = 30;
    int num_active_white_features;
    int num_active_black_features;

    std::unique_ptr<float[]> side_to_move;
    std::unique_ptr<float[]> evaluation;
    std::unique_ptr<int[]> white_features;
    std::unique_ptr<int[]> black_features;

public:
    SparseBatch(int batch_size);

    void fill(Dataset& dataset);

    float* getSideToMove() const;
    float* getEvaluation() const;
    int* getWhiteFeatures() const;
    int* getBlackFeatures() const;

    int getNumActiveWhiteFeatures() const;
    int getNumActiveBlackFeatures() const;
};