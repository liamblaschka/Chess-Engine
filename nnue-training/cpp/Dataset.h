#pragma once

#include "TrainingEntry.h"
#include <string>
#include <vector>
#include <random>
#include <atomic>

class Dataset {
private:
    std::vector<TrainingEntry> data;
    
    std::atomic<int> next_batch_start;
    
    std::mt19937 rng;
    
    std::uniform_real_distribution<double> prob_distribution;

    std::vector<int> shuffled_indices;

    bool use_data_augmentation;

public:
    Dataset(const std::string& file_path, bool use_data_augmentation);

    void readCSV(const std::string& file_path);
    void flipFenPerspective(std::string& fen) const;
    float parseEvaluation(const std::string& evaluation) const;

    int getNextBatchStart(int batch_size);
    const TrainingEntry& getEntry(int index);

    int getDataSize() const;

    void resetEpoch();
};