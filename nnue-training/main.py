from model import NNUE
from dataset import PositionDataset
from train import train
import torch
from torch.utils.data import DataLoader
import numpy as np

BATCH_SIZE = 256

def main():
    dataset = PositionDataset("train_data/chessData.csv")
    
    model = NNUE()
    
    dataloader = DataLoader(dataset, batch_size=BATCH_SIZE, shuffle=True)
    
    train(model, dataloader)
    
    torch.save(model.state_dict(), "nnue.pth")
    
    
    with open("nnue.bin", "wb") as f:
        for layer in [model.accumulator, model.h1, model.output]:
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
