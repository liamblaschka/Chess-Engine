import torch
import torch.nn as nn
import numpy as np
import os
import time
from model import NNUE
from data_loader import Dataset
from data_loader import DataLoader

ACTIVATION_SCALE = 127
WEIGHT_SCALE = 64

DATA_SIZE = 12_000_000
BATCH_SIZE = 2048
DATALOADER_WORKERS = 12

LEARNING_RATE = 0.001
EPOCHS = 60

def train(model: NNUE, dataloader: DataLoader, epochs=EPOCHS, learning_rate=LEARNING_RATE):
    if torch.accelerator.is_available():
        device = torch.accelerator.current_accelerator()
    else:
        device = torch.device("cpu")

    print(f"Using device: {device}")

    model.to(device)
    
    criterion = nn.BCEWithLogitsLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
    
    model.train()
    
    for epoch in range(epochs):
        start_time = time.perf_counter()
        
        total_loss = 0.0
        
        for _ in range(dataloader.num_batches):
            batch = dataloader.batch
            
            batch.half_kp.white = batch.half_kp.white.to(device)
            batch.half_kp.black = batch.half_kp.black.to(device)
            if dataloader.fill_virtual_features:
                batch.half_relative_kp.white = batch.half_relative_kp.white.to(device)
                batch.half_relative_kp.black = batch.half_relative_kp.black.to(device)
                batch.king_factor.white = batch.king_factor.white.to(device)
                batch.king_factor.black = batch.king_factor.black.to(device)
            batch.side_to_move = batch.side_to_move.to(device)
            batch.evaluation = batch.evaluation.to(device)
            
            optimizer.zero_grad() # clear previous batch gradients
            
            if dataloader.fill_virtual_features:
                predictions = model(
                    batch.side_to_move,
                    batch.half_kp.white, batch.half_kp.black,
                    batch.half_relative_kp.white, batch.half_relative_kp.black,
                    batch.king_factor.white, batch.king_factor.black
                )
            else:
                predictions = model(
                    batch.side_to_move,
                    batch.half_kp.white, batch.half_kp.black
                )
            
            wdl_scale = 410
            wdl_targets = torch.sigmoid(batch.evaluation / wdl_scale)
            
            loss = criterion(predictions, wdl_targets)
            
            loss.backward() # backpropagation (figure out our loss)
            optimizer.step() # update the weights with optimizer
            
            model.clip_weights()
            
            total_loss += loss.item()
        
        
        dataloader.reset_epoch()
        
        
        average_loss = total_loss / dataloader.num_batches
        print(f"Epoch {epoch+1}: loss {average_loss:.6f}, time {time.perf_counter() - start_time:.2f}s")
    
    
    
    if dataloader.fill_virtual_features:
        model.coalesce_weights()






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
