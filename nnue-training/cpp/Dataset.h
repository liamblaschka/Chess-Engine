#pragma once

#include "TrainingEntry.h"
#include <string>
#include <vector>
#include <random>

class Dataset {
private:
    std::vector<TrainingEntry> data;
    
    int shuffled_index;
    std::mt19937 rng;
    std::vector<int> shuffled_indices;

public:
    Dataset(const std::string& file_path, int data_size);

    void readCSV(const std::string& file_path, int data_size);
    float parseEvaluation(const std::string& evaluation) const;

    void resetEpoch();

    const TrainingEntry& getNextEntry();
};