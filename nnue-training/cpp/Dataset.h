#pragma once

#include "TrainingEntry.h"
#include <string>
#include <vector>
#include <random>
#include <atomic>

class Dataset {
private:
    std::vector<TrainingEntry> data;

    int data_size;
    
    std::atomic<int> next_batch_start;
    std::mt19937 rng;
    std::vector<int> shuffled_indices;

public:
    Dataset(const std::string& file_path, int data_size);

    void readCSV(const std::string& file_path, int data_size);
    float parseEvaluation(const std::string& evaluation) const;

    int getNextBatchStart(int batch_size);
    const TrainingEntry& getEntry(int index);

    int getDataSize() const;

    void resetEpoch();
};