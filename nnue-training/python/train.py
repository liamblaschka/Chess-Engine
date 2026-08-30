import torch
import torch.nn as nn
import time
from model import NNUE
from data_loader import DataLoader
from data_loader import SparseBatch

LEARNING_RATE = 0.001
EPOCHS = 10

def train(model: NNUE, dataloader: DataLoader, epochs=EPOCHS, learning_rate=LEARNING_RATE):
    if torch.accelerator.is_available():
        device = torch.accelerator.current_accelerator()
    else:
        device = torch.device("cpu")

    print(f"Using device: {device}")

    model.to(device)
    
    criterion = nn.MSELoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
    
    model.train()
    
    for epoch in range(epochs):
        start_time = time.perf_counter()
        
        total_loss = 0.0
        
        for _ in range(dataloader.num_batches):
            dataloader.fill_batch()
            
            white_features, black_features, side_to_move, evaluation = dataloader.batch.tensors
            
            white_features = white_features.to(device)
            black_features = black_features.to(device)
            side_to_move = side_to_move.to(device)
            evaluation = evaluation.to(device)
            
            optimizer.zero_grad() # clear previous batch gradients
            
            predictions = model(white_features, black_features, side_to_move)
            
            loss = criterion(predictions, evaluation)
            
            loss.backward() # backpropagation (figure out our loss)
            optimizer.step() # update the weights with optimizer
            
            total_loss += loss.item()
            
        dataloader.reset_epoch()
            
        average_loss = total_loss / dataloader.num_batches
        print(f"Epoch {epoch+1}: loss {average_loss:.6f}, time {time.perf_counter() - start_time:.2f}s")
