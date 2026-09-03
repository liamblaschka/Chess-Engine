#pragma once

#include <memory>

class SparseFeatures {
private:
    int num_active_white;
    int num_active_black;

    std::unique_ptr<int[]> white_indices;
    std::unique_ptr<int[]> black_indices;

public:
    SparseFeatures(int batch_size, int max_active_features);

    void addWhiteFeature(int entry_index, int feature);
    void addBlackFeature(int entry_index, int feature);

    void reset();

    int* getWhiteIndices() const;
    int* getBlackIndices() const;

    int getNumActiveWhite() const;
    int getNumActiveBlack() const;
};