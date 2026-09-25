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

    int getNumActiveWhiteFeatures() const;
    int getNumActiveBlackFeatures() const;
};