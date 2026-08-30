#include "DataLoader.h"
#include "Dataset.h"
#include "SparseBatch.h"

DataLoader::DataLoader(Dataset& dataset, int batch_size, int num_workers)
    : dataset(dataset), batch(batch_size), batch_size(batch_size), num_workers(num_workers) {}

void DataLoader::fillBatch() {
    batch.fill(dataset);
}

const SparseBatch& DataLoader::getBatch() const {
    return batch;
}

void DataLoader::resetEpoch() {
    dataset.resetEpoch();
}


extern "C" {
    DataLoader* DataLoader_new(Dataset* dataset, int batch_size, int num_workers) {
        return new DataLoader(*dataset, batch_size, num_workers);
    }

    void DataLoader_delete(DataLoader* data_loader) {
        delete data_loader;
    }

    void DataLoader_fillBatch(DataLoader* data_loader) {
        data_loader->fillBatch();
    }

    const SparseBatch* DataLoader_getBatch(const DataLoader* data_loader) { return &data_loader->getBatch(); }

    void DataLoader_resetEpoch(DataLoader* data_loader) {
        data_loader->resetEpoch();
    }
}