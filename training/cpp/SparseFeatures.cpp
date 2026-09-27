#include "SparseFeatures.h"
#include <memory>

SparseFeatures::SparseFeatures(int batch_size, int max_active_features) {
    num_active_white = 0;
    num_active_black = 0;
    white_indices = std::make_unique<int[]>(batch_size * max_active_features * 2);
    black_indices = std::make_unique<int[]>(batch_size * max_active_features * 2);
}

void SparseFeatures::addWhiteFeature(int entry_index, int feature) {
    int offset = num_active_white * 2;

    white_indices[offset] = entry_index;
    white_indices[offset + 1] = feature;

    num_active_white++;
}

void SparseFeatures::addBlackFeature(int entry_index, int feature) {
   int offset = num_active_black * 2;

    black_indices[offset] = entry_index;
    black_indices[offset + 1] = feature;
    
    num_active_black++;
}

void SparseFeatures::reset() {
    num_active_white = 0;
    num_active_black = 0;
}

int* SparseFeatures::getWhiteIndices() const {
    return white_indices.get();
}

int* SparseFeatures::getBlackIndices() const {
    return black_indices.get();
}

int SparseFeatures::getNumActiveWhite() const {
    return num_active_white;
}

int SparseFeatures::getNumActiveBlack() const {
    return num_active_black;
}


extern "C" {
    int* SparseFeatures_getWhiteIndices(const SparseFeatures* features) { return features->getWhiteIndices(); }
    
    int* SparseFeatures_getBlackIndices(const SparseFeatures* features) { return features->getBlackIndices(); }

    int SparseFeatures_getNumActiveWhite(const SparseFeatures* features) { return features->getNumActiveWhite(); }

    int SparseFeatures_getNumActiveBlack(const SparseFeatures* features) { return features->getNumActiveBlack(); }
}