#pragma once

#include "Dataset.h"

class SparseBatch {
private:
    int size;
    
    static constexpr int MAX_FEATURES = 40960;
    int num_active_white_features;
    int num_active_black_features;

    float* side_to_move;
    float* evaluation;
    int* white_features;
    int* black_features;

public:
    SparseBatch(int batch_size);

    void fill(Dataset& dataset);

    float* getSideToMove() const;
    float* getEvaluation() const;
    int* getWhiteFeatures() const;
    int* getBlackFeatures() const;

    int getNumActiveWhiteFeatures() const;
    int getNumActiveBlackFeatures() const;
    
    ~SparseBatch();
};