import torch
import numpy as np
import os
from model import NNUE
from data_loader import Dataset
from data_loader import DataLoader
from train import train

DATA_SIZE = 12_000_000
BATCH_SIZE = 2048
DATALOADER_WORKERS = 12

ACTIVATION_SCALE = 127
WEIGHT_SCALE = 64


def quantize_accumulator(layer):
    weights = torch.round(layer.weight.detach() * ACTIVATION_SCALE)
    biases = torch.round(layer.bias.detach() * ACTIVATION_SCALE)

    if weights.min() < -32768 or weights.max() > 32767:
        raise ValueError(f"Accumulator weights (min: {weights.min()}, max: {weights.max()}) outside int16 range")
    if biases.min() < -32768 or biases.max() > 32767:
        raise ValueError(f"Accumulator weights (min: {weights.min()}, max: {weights.max()}) outside int16 range")

    weights = weights.cpu().numpy().astype(np.int16).T
    biases = biases.cpu().numpy().astype(np.int16)

    return weights, biases

def quantize_hidden_layer(layer):
    weights = torch.round(layer.weight.detach() * WEIGHT_SCALE)
    biases = torch.round(layer.bias.detach() * ACTIVATION_SCALE * WEIGHT_SCALE)

    if weights.min() < -128 or weights.max() > 127:
        raise ValueError(f"Hidden weights (min: {weights.min()}, max: {weights.max()}) outside int8 range")

    weights = weights.cpu().numpy().astype(np.int8).T
    biases = biases.cpu().numpy().astype(np.int32)

    return weights, biases

def write_layer(file, weights, biases):
    file.write(weights.tobytes())
    file.write(biases.tobytes())

def export_quantized_model(model, path):
    with open(path, "wb") as file:
        for layer in (model.w_half_kp, model.b_half_kp):
            weights, biases = quantize_accumulator(layer)
            write_layer(file, weights, biases)

        for layer in (model.h1, model.h2, model.output):
            weights, biases = quantize_hidden_layer(layer)
            write_layer(file, weights, biases)


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    training_dir = os.path.dirname(script_dir)
    project_dir = os.path.dirname(training_dir)

    data_path = os.path.join(training_dir, "data", "chessData.csv")
    models_dir = os.path.join(project_dir, "models")

    os.makedirs(models_dir, exist_ok=True)

    dataset = Dataset(data_path, DATA_SIZE)
    dataloader = DataLoader(dataset, BATCH_SIZE, DATALOADER_WORKERS)

    model = NNUE()
    train(model, dataloader)


    pth_path = os.path.join(models_dir, "nnue.pth")
    torch.save(model.state_dict(), pth_path)
    
    bin_path = os.path.join(models_dir, "nnue.bin")
    export_quantized_model(model, bin_path)


if __name__ == "__main__":
    main()
