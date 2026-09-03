#pragma once

#include "Dataset.h"
#include "SparseBatch.h"
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include <condition_variable>
#include <atomic>

class DataLoader {
private:
    Dataset& dataset;
    int batch_size;
    int num_workers;
    int required_batches;
    int remaining_batches;
    bool fill_virtual_features;

    std::atomic<bool> running;
    std::mutex remaining_mutex;
    std::condition_variable remaining_cv;
    std::mutex available_mutex;
    std::condition_variable available_cv;
    std::mutex ready_mutex;
    std::condition_variable ready_cv;

    std::unique_ptr<SparseBatch> current_batch;
    std::queue<std::unique_ptr<SparseBatch>> available_batches;
    std::queue<std::unique_ptr<SparseBatch>> ready_batches;
    std::vector<std::thread> workers;

public:
    DataLoader(Dataset& dataset, int batch_size, int num_workers, bool fill_virtual_features);

    void workerLoop();

    const SparseBatch* getBatch();

    void setFillVirtualFeatures(bool value);

    void resetEpoch();

    ~DataLoader();
};