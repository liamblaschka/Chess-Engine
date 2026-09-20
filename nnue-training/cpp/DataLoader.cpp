#include "DataLoader.h"
#include "Dataset.h"
#include "SparseBatch.h"
#include <vector>
#include <thread>
#include <memory>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <atomic>

DataLoader::DataLoader(Dataset& dataset, int batch_size, int num_workers, bool fill_virtual_features)
    : dataset(dataset), batch_size(batch_size), num_workers(num_workers), fill_virtual_features(fill_virtual_features), running(true)
{
    required_batches = dataset.getDataSize() / batch_size;
    remaining_batches = required_batches;

    for (int i = 0; i < num_workers + 1; i++) {
        available_batches.push(std::make_unique<SparseBatch>(batch_size));
    }

    for (int i = 0; i < num_workers; i++) {
        workers.emplace_back(&DataLoader::workerLoop, this);
    }
}

void DataLoader::workerLoop() {
    while (running) {
        {
            std::unique_lock<std::mutex> lock(remaining_mutex);
            while (remaining_batches <= 0 && running) {
                remaining_cv.wait(lock);
            }
            if (!running) {
                return;
            }

            remaining_batches--;
        }


        std::unique_ptr<SparseBatch> batch;
        
        {
            std::unique_lock<std::mutex> lock(available_mutex);

            while (available_batches.empty() && running) {
                available_cv.wait(lock);
            }
            if (!running) {
                return;
            }

            batch = std::move(available_batches.front());
            available_batches.pop();
        }

        batch->fill(dataset, fill_virtual_features);

        {
            std::lock_guard<std::mutex> lock(ready_mutex);

            ready_batches.push(std::move(batch));
        }
        ready_cv.notify_one();
    }
}

const SparseBatch* DataLoader::getBatch() {
    if (current_batch) {
        {
            std::lock_guard<std::mutex> lock(available_mutex);
            available_batches.push(std::move(current_batch));
        }
        available_cv.notify_one();
    }

    {
        std::unique_lock<std::mutex> lock(ready_mutex);

        while (ready_batches.empty()) {
            ready_cv.wait(lock);
        }

        current_batch = std::move(ready_batches.front());
        ready_batches.pop();
    }

    return current_batch.get();
}

void DataLoader::setFillVirtualFeatures(bool value) {
    fill_virtual_features = value;
}

void DataLoader::resetEpoch() {
    if (current_batch) {
        std::lock_guard<std::mutex> lock(available_mutex);
        available_batches.push(std::move(current_batch));
    }

    dataset.resetEpoch();

    {
        std::lock_guard<std::mutex> lock(remaining_mutex);
        remaining_batches = required_batches;
    }
    remaining_cv.notify_all();
}

DataLoader::~DataLoader() {
    running = false;

    remaining_cv.notify_all();
    available_cv.notify_all();

    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}


extern "C" {
    DataLoader* DataLoader_new(Dataset* dataset, int batch_size, int num_workers, bool fill_virtual_features) {
        return new DataLoader(*dataset, batch_size, num_workers, fill_virtual_features);
    }

    void DataLoader_delete(DataLoader* data_loader) {
        delete data_loader;
    }

    const SparseBatch* DataLoader_getBatch(DataLoader* data_loader) { return data_loader->getBatch(); }

    void DataLoader_setFillVirtualFeatures(DataLoader* data_loader, bool value) {
        data_loader->setFillVirtualFeatures(value);
    }

    void DataLoader_resetEpoch(DataLoader* data_loader) {
        data_loader->resetEpoch();
    }
}