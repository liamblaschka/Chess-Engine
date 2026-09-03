import torch
import numpy as np
import os
from model import NNUE
from data_loader import Dataset
from data_loader import DataLoader
from train import train

DATA_SIZE = 5_000_000
BATCH_SIZE = 512

DATALOADER_WORKERS = 12

def main():
    file_name = "chessData.csv"
    script_dir = os.path.dirname(os.path.abspath(__file__))
    training_dir = os.path.dirname(script_dir)
    data_path = os.path.join(training_dir, "data", file_name)
    
    dataset = Dataset(data_path, DATA_SIZE)
    
    dataloader = DataLoader(dataset, BATCH_SIZE, DATALOADER_WORKERS)
    
    model = NNUE()
    
    
    train(model, dataloader)
    
    
    project_dir = os.path.dirname(training_dir)
    models_dir = os.path.join(project_dir, "models")
    os.makedirs(models_dir, exist_ok=True)
    pth_path = os.path.join(models_dir, "nnue.pth")
    bin_path = os.path.join(models_dir, "nnue.bin")

    torch.save(model.state_dict(), pth_path)

    with open(bin_path, "wb") as f:
        for layer in [model.w_half_kp, model.b_half_kp, model.h1, model.h2, model.output]:
            weights = (
                layer.weight
                .detach()
                .cpu()
                .numpy()
                .astype(np.float32)
                .T
            )

            biases = (
                layer.bias
                .detach()
                .cpu()
                .numpy()
                .astype(np.float32)
            )

            f.write(weights.tobytes())
            f.write(biases.tobytes())
        
    
if __name__ == "__main__":
    main()
