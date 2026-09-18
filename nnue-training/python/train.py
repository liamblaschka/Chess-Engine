import torch
import torch.nn as nn
import time
from model import NNUE
from data_loader import DataLoader

LEARNING_RATE = 0.001
EPOCHS = 10

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
        
        # if epoch == 4:
        #     model.coalesce_weights()
        #     dataloader.fill_virtual_features = False
            
        #     optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
        
        dataloader.reset_epoch()
        
        
        average_loss = total_loss / dataloader.num_batches
        print(f"Epoch {epoch+1}: loss {average_loss:.6f}, time {time.perf_counter() - start_time:.2f}s")
    
    
    
    if dataloader.fill_virtual_features:
        model.coalesce_weights()
