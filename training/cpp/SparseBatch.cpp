#include "Dataset.h"
#include "TrainingEntry.h"
#include "SparseFeatures.h"
#include "SparseBatch.h"
#include <vector>
#include <algorithm>
#include <memory>

SparseBatch::SparseBatch(int size) : size(size), half_kp(size, 30), half_relative_kp(size, 30), king_factor(size, 30) {
    side_to_move = std::make_unique<float[]>(size);
    evaluation = std::make_unique<float[]>(size);
}

void SparseBatch::fill(Dataset& dataset, bool fill_virtual_features) {
    half_kp.reset();
    if (fill_virtual_features) {
        half_relative_kp.reset();
        king_factor.reset();
    }

    int batch_start = dataset.getNextBatchStart(size);

    for (int entry_index = 0; entry_index < size; entry_index++) {
        const TrainingEntry& entry = dataset.getEntry(batch_start + entry_index);

        side_to_move[entry_index] = static_cast<float>(entry.side_to_move);
        evaluation[entry_index] = entry.evaluation;
        
        std::vector<int> white_halfkp_indices;
        std::vector<int> black_halfkp_indices;

        std::vector<int> white_relative_indices;
        std::vector<int> black_relative_indices;

        std::vector<int> white_king_indices;
        std::vector<int> black_king_indices;
        for (const Piece& piece : entry.pieces) {
            white_halfkp_indices.push_back(getHalfKPIndex(piece, entry.white_king_square));
            black_halfkp_indices.push_back(getHalfKPIndex(piece, entry.black_king_square));
            if (fill_virtual_features) {
                white_relative_indices.push_back(getHalfRelativeKPIndex(piece, entry.white_king_square));
                black_relative_indices.push_back(getHalfRelativeKPIndex(piece, entry.black_king_square));

                white_king_indices.push_back(getKingFactorIndex(entry.white_king_square));
                black_king_indices.push_back(getKingFactorIndex(entry.black_king_square));
            }
        }
        
        // Add position entry index, features (ascending indices) to sparse batch
        std::sort(white_halfkp_indices.begin(), white_halfkp_indices.end());
        std::sort(black_halfkp_indices.begin(), black_halfkp_indices.end());
        if (fill_virtual_features) {
            std::sort(white_relative_indices.begin(), white_relative_indices.end());
            std::sort(black_relative_indices.begin(), black_relative_indices.end());
        }

        addFeatures(half_kp, entry_index, white_halfkp_indices, black_halfkp_indices);
        if (fill_virtual_features) {
            addFeatures(half_relative_kp, entry_index, white_relative_indices, black_relative_indices);
            addFeatures(king_factor, entry_index, white_king_indices, black_king_indices);
        }
    }
}

void SparseBatch::addFeatures(
    SparseFeatures& features, int entry_index,
    const std::vector<int>& white_indices, const std::vector<int>& black_indices)
{
    for (int i = 0; i < white_indices.size(); i++) {
        features.addWhiteFeature(entry_index, white_indices[i]);
    }

    for (int i = 0; i < black_indices.size(); i++) {
        features.addBlackFeature(entry_index, black_indices[i]);
    }
}

int SparseBatch::getHalfKPIndex(const Piece& piece, int king_square) const {
    int p_idx = static_cast<int>(piece.type) * 2 + static_cast<int>(piece.side);
    int half_kp_idx = piece.square + (p_idx + king_square * 10) * 64;

    return half_kp_idx;
}

int SparseBatch::getHalfRelativeKPIndex(const Piece& piece, int king_square) const {
    int piece_rank = piece.square / 8;
    int piece_file = piece.square % 8;
    int king_rank = king_square / 8;
    int king_file = king_square % 8;

    int p_idx = static_cast<int>(piece.type) * 2 + static_cast<int>(piece.side);

    // Difference is -7..7, need to map to 0..14 to use as indices
    int relative_file = piece_file - king_file + 7;
    int relative_rank = piece_rank - king_rank + 7;

    int relative_square = relative_rank * 15 + relative_file;

    // 15 * 15 relative positions for each piece
    return (p_idx * 15 * 15) + relative_square;
}

int SparseBatch::getKingFactorIndex(int king_square) const {
    return king_square;
}

float* SparseBatch::getSideToMove() const { return side_to_move.get(); }

float* SparseBatch::getEvaluation() const { return evaluation.get(); }

const SparseFeatures* SparseBatch::getHalfKPFeatures() const { return &half_kp; }

const SparseFeatures* SparseBatch::getHalfRelativeKPFeatures() const { return &half_relative_kp; }

const SparseFeatures* SparseBatch::getKingFeatures() const { return &king_factor; }


extern "C" {
    float* SparseBatch_getSideToMove(const SparseBatch* batch) { return batch->getSideToMove(); }

    float* SparseBatch_getEvaluation(const SparseBatch* batch) { return batch->getEvaluation(); }

    const SparseFeatures* SparseBatch_getHalfKPFeatures(const SparseBatch* batch) { return batch->getHalfKPFeatures(); }

    const SparseFeatures* SparseBatch_getHalfRelativeKPFeatures(const SparseBatch* batch) { return batch->getHalfRelativeKPFeatures(); }

    const SparseFeatures* SparseBatch_getKingFeatures(const SparseBatch* batch) { return batch->getKingFeatures(); }
}