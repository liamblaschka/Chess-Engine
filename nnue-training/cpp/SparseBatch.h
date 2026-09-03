#pragma once

#include "Dataset.h"
#include "TrainingEntry.h"
#include "SparseFeatures.h"
#include <memory>
#include <vector>

class SparseBatch {
private:
    int size;

    std::unique_ptr<float[]> side_to_move;
    std::unique_ptr<float[]> evaluation;

    SparseFeatures half_kp;
    SparseFeatures half_relative_kp;
    SparseFeatures king_factor;
 

    // static constexpr int MAX_ACTIVE_FEATURES = 30;
    // int num_active_white_features;
    // int num_active_black_features;
    // std::unique_ptr<int[]> white_features;
    // std::unique_ptr<int[]> black_features;

    // static constexpr int MAX_ACTIVE_RELATIVE_FEATURES = 0;
    // int num_active_white_relative_features;
    // int num_active_black_relative_features;
    // std::unique_ptr<int[]> white_relative_features;
    // std::unique_ptr<int[]> black_relative_features;

    // static constexpr int MAX_ACTIVE_K_FEATURES = 1;
    // int num_active_white_k_features;
    // int num_active_black_k_features;
    // std::unique_ptr<int[]> white_k_features;
    // std::unique_ptr<int[]> black_k_features;

public:
    SparseBatch(int batch_size);

    void fill(Dataset& dataset, bool fill_virutal_features);

    void addFeatures(
        SparseFeatures& features, int entry_index,
        const std::vector<int>& white_indices, const std::vector<int>& black_indices);

    int getHalfKPIndex(const Piece& piece, int king_square) const;
    int getHalfRelativeKPIndex(const Piece& piece, int king_square) const;
    int getKingFactorIndex(int king_square) const;

    float* getSideToMove() const;
    float* getEvaluation() const;

    const SparseFeatures* getHalfKPFeatures() const;
    const SparseFeatures* getHalfRelativeKPFeatures() const;
    const SparseFeatures* getKingFeatures() const;


    // int* getWhiteHalfKPFeatures() const;
    // int* getBlackHalfKPFeatures() const;
    // int* getWhiteHalfRelativeKPFeatures() const;
    // int* getBlackHalfRelativeKPFeatures() const;
    // int* getWhiteKingFeatures() const;
    // int* getBlackKingFeatures() const;

    int getNumActiveWhiteFeatures() const;
    int getNumActiveBlackFeatures() const;
};