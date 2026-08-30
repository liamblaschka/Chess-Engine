#pragma once

#include "Dataset.h"
#include "SparseBatch.h"

class DataLoader {
private:
    Dataset dataset;
    SparseBatch batch;
    int batch_size;
    int num_workers;

public:
    DataLoader(Dataset& dataset, int batch_size, int num_workers);

    void fillBatch();
    const SparseBatch& getBatch() const;

    void resetEpoch();
};