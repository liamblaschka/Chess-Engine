#include "SparseBatch.h"
#include "Dataset.h"
#include "TrainingEntry.h"
#include <vector>
#include <cstdint>
#include <algorithm>

SparseBatch::SparseBatch(int size)  : size(size), num_active_white_features(0), num_active_black_features(0) {    
    side_to_move = new float[size];
    evaluation = new float[size];

    white_features = new int[size * MAX_FEATURES * 2];
    black_features = new int[size * MAX_FEATURES * 2];
}

void SparseBatch::fill(Dataset& dataset) {
    num_active_white_features = 0;
    num_active_black_features = 0;

    for (int entry_index = 0; entry_index < size; entry_index++) {
        const TrainingEntry& entry = dataset.getNextEntry();

        side_to_move[entry_index] = static_cast<float>(entry.side_to_move);
        evaluation[entry_index] = entry.evaluation;
        
        std::vector<int> white_feature_indices;
        std::vector<int> black_feature_indices;
        for (const Piece& piece : entry.pieces) {
            int p_idx = static_cast<int>(piece.type) * 2 + static_cast<int>(piece.side);
            int white_halfkp_idx = piece.square + (p_idx + entry.white_king_square * 10) * 64;
            int black_halfkp_idx = piece.square + (p_idx + entry.black_king_square * 10) * 64;

            white_feature_indices.push_back(white_halfkp_idx);
            black_feature_indices.push_back(black_halfkp_idx);
        }
        
        // Add position entry index, halfkp index (ascending indices) to sparse batch
        std::sort(white_feature_indices.begin(), white_feature_indices.end());
        std::sort(black_feature_indices.begin(), black_feature_indices.end());
        for (int i = 0; i < white_feature_indices.size(); i++) {
            int offset = num_active_white_features * 2;
            
            white_features[offset] = entry_index;
            white_features[offset + 1] = white_feature_indices[i];
            num_active_white_features++;

            black_features[offset] = entry_index;
            black_features[offset + 1] = black_feature_indices[i];
            num_active_black_features++;
        }
    }
}

float* SparseBatch::getSideToMove() const { return side_to_move; }

float* SparseBatch::getEvaluation() const { return evaluation; }

int* SparseBatch::getWhiteFeatures() const { return white_features; }

int* SparseBatch::getBlackFeatures() const { return black_features; }

int SparseBatch::getNumActiveWhiteFeatures() const { return num_active_white_features; }

int SparseBatch::getNumActiveBlackFeatures() const { return num_active_black_features; }

SparseBatch::~SparseBatch() {
    delete[] side_to_move;
    delete[] evaluation;
    delete[] white_features;
    delete[] black_features;
}


extern "C" {
    float* SparseBatch_getSideToMove(const SparseBatch* batch) { return batch->getSideToMove(); }

    float* SparseBatch_getEvaluation(const SparseBatch* batch) { return batch->getEvaluation(); }

    int* SparseBatch_getWhiteFeatures(const SparseBatch* batch) { return batch->getWhiteFeatures(); }

    int* SparseBatch_getBlackFeatures(const SparseBatch* batch) { return batch->getBlackFeatures(); }

    int SparseBatch_getNumActiveWhiteFeatures(const SparseBatch* batch) { return batch->getNumActiveWhiteFeatures(); }

    int SparseBatch_getNumActiveBlackFeatures(const SparseBatch* batch) { return batch->getNumActiveBlackFeatures(); }
}