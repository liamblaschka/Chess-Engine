import torch
import torch.nn as nn
from torch.utils.data import DataLoader
import time
from model import NNUE

LEARNING_RATE = 0.001
EPOCHS = 10

def train(model: NNUE, dataloader: DataLoader, epochs=EPOCHS, learning_rate=LEARNING_RATE):
    criterion = nn.MSELoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
    
    model.train()
    
    for epoch in range(epochs):
        start_time = time.perf_counter()
        
        total_loss = 0.0
        
        for features, targets, side_to_move in dataloader:
            predictions = model(features, side_to_move).squeeze(1)
            
            loss = criterion(predictions, targets)
            
            optimizer.zero_grad() # clear previous batch gradients
            loss.backward() # backpropagation (figure out our loss)
            optimizer.step() # update the weights with optimizer
            
            total_loss += loss.item()
            
        average_loss = total_loss / len(dataloader)
        print(f"Epoch {epoch+1}: loss {average_loss:.6f}, time {time.perf_counter() - start_time:.2f}s")
