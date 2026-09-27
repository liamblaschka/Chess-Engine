import torch
import torch.nn as nn
import numpy as np
import os
import time
import csv

from model import NNUE
from data_loader import Dataset
from data_loader import DataLoader


ACTIVATION_SCALE = 127
WEIGHT_SCALE = 64
WDL_SCALE = 410

BATCH_SIZE = 2048
LEARNING_RATE = 0.002
EPOCHS = 60

EARLY_STOPPING_PATIENCE = 7


def move_batch_to_device(batch, device, fill_virtual_features):
    batch.half_kp.white = batch.half_kp.white.to(device)
    batch.half_kp.black = batch.half_kp.black.to(device)

    if fill_virtual_features:
        batch.half_relative_kp.white = batch.half_relative_kp.white.to(device)
        batch.half_relative_kp.black = batch.half_relative_kp.black.to(device)
        batch.king_factor.white = batch.king_factor.white.to(device)
        batch.king_factor.black = batch.king_factor.black.to(device)

    batch.side_to_move = batch.side_to_move.to(device)
    batch.evaluation = batch.evaluation.to(device)


def forward(model, batch, fill_virtual_features):
    if fill_virtual_features:
        return model(batch.side_to_move, batch.half_kp.white, batch.half_kp.black,
                     batch.half_relative_kp.white, batch.half_relative_kp.black,
                     batch.king_factor.white, batch.king_factor.black)

    return model(batch.side_to_move, batch.half_kp.white, batch.half_kp.black)


def validate(model: NNUE, dataloader: DataLoader, criterion, device):
    model.eval()
    total_loss = 0.0

    with torch.no_grad():
        for _ in range(dataloader.num_batches):
            batch = dataloader.batch

            move_batch_to_device(batch, device, dataloader.fill_virtual_features)

            predictions = forward(model, batch, dataloader.fill_virtual_features)

            wdl_targets = torch.sigmoid(batch.evaluation / WDL_SCALE)
            loss = criterion(predictions, wdl_targets)

            total_loss += loss.item()

    average_loss = total_loss / dataloader.num_batches

    dataloader.reset_epoch()

    return average_loss


def train(model: NNUE, train_loader: DataLoader, validation_loader: DataLoader, models_dir, log_path,
          epochs=EPOCHS, learning_rate=LEARNING_RATE):

    if torch.accelerator.is_available():
        device = torch.accelerator.current_accelerator()
    else:
        device = torch.device("cpu")

    print(f"Using device: {device}")

    model.to(device)

    criterion = nn.BCEWithLogitsLoss()
    optimizer = torch.optim.Adam(model.parameters(), lr=learning_rate)
    
    scheduler = torch.optim.lr_scheduler.ReduceLROnPlateau(
        optimizer,
        mode="min",
        factor=0.5,
        patience=2,
        min_lr=0.00005
    )

    best_validation_loss = float("inf")
    
    epochs_without_improvement = 0

    latest_path = os.path.join(models_dir, "latest.pth")
    best_path = os.path.join(models_dir, "best.pth")

    with open(log_path, "w", newline="") as log_file:
        writer = csv.writer(log_file)
        writer.writerow(["epoch", "training_loss", "validation_loss", "time_seconds"])

        for epoch in range(epochs):
            start_time = time.perf_counter()

            # Training
            model.train()
            total_loss = 0.0

            for _ in range(train_loader.num_batches):
                batch = train_loader.batch

                move_batch_to_device(batch, device, train_loader.fill_virtual_features)

                optimizer.zero_grad()

                predictions = forward(model, batch, train_loader.fill_virtual_features)

                wdl_targets = torch.sigmoid(batch.evaluation / WDL_SCALE)
                loss = criterion(predictions, wdl_targets)

                loss.backward()
                optimizer.step()

                model.clip_weights()

                total_loss += loss.item()

            training_loss = total_loss / train_loader.num_batches

            train_loader.reset_epoch()

            # Validation
            validation_loss = validate(model, validation_loader, criterion, device)
            
            epoch_lr = optimizer.param_groups[0]["lr"]
            scheduler.step(validation_loss)

            epoch_time = time.perf_counter() - start_time

            print(f"Epoch {epoch + 1}: train_loss={training_loss:.6f}, "
                  f"validation_loss={validation_loss:.6f}, lr={epoch_lr:.6f}, time={epoch_time:.2f}s")
            

            # Log losses
            writer.writerow([epoch + 1, training_loss, validation_loss, epoch_time])
            log_file.flush()

            # Save latest checkpoint
            checkpoint = {
                "epoch": epoch + 1,
                "model_state_dict": model.state_dict(),
                "optimizer_state_dict": optimizer.state_dict(),
                "training_loss": training_loss,
                "validation_loss": validation_loss
            }

            torch.save(checkpoint, latest_path)

            # Save best checkpoint
            if validation_loss < best_validation_loss:
                best_validation_loss = validation_loss
                torch.save(checkpoint, best_path)
                epochs_without_improvement = 0
                
                print(f"New best model (validation_loss={validation_loss:.6f})")
            else:
                epochs_without_improvement += 1
                
            if epochs_without_improvement >= EARLY_STOPPING_PATIENCE:
                print(f"\nEarly stop. No improvement for {epochs_without_improvement} epochs.")
                break

    print(f"Best validation loss: {best_validation_loss:.6f}")

    return best_path


def quantize_accumulator(layer):
    weights = torch.round(layer.weight.detach() * ACTIVATION_SCALE)
    biases = torch.round(layer.bias.detach() * ACTIVATION_SCALE)

    if weights.min() < -32768 or weights.max() > 32767:
        raise ValueError(f"Accumulator weights (min: {weights.min()}, max: {weights.max()}) outside int16 range")

    if biases.min() < -32768 or biases.max() > 32767:
        raise ValueError(f"Accumulator biases (min: {biases.min()}, max: {biases.max()}) outside int16 range")

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

    train_path = os.path.join(training_dir, "data", "train.csv")
    validation_path = os.path.join(training_dir, "data", "validation.csv")
    models_dir = os.path.join(project_dir, "models")
    log_path = os.path.join(models_dir, "training_log.csv")

    os.makedirs(models_dir, exist_ok=True)

    train_dataset = Dataset(train_path)
    print(f"Training data size: {train_dataset.size:,}")
    
    validation_dataset = Dataset(validation_path, use_data_augmentation=False)
    print(f"Validation data size: {validation_dataset.size:,}")

    train_loader = DataLoader(train_dataset, BATCH_SIZE)
    validation_loader = DataLoader(validation_dataset, BATCH_SIZE)

    model = NNUE()

    best_path = train(model, train_loader, validation_loader, models_dir, log_path)

    # Load the model from the epoch with the lowest validation loss.
    checkpoint = torch.load(best_path, map_location="cpu")
    model.load_state_dict(checkpoint["model_state_dict"])
    model.to("cpu")

    print(f"Using model from epoch {checkpoint['epoch']} with validation loss {checkpoint['validation_loss']:.6f}")

    # Convert virtual features into the normal HalfKP weights before final export.
    if train_loader.fill_virtual_features:
        model.coalesce_weights()

    # Save final PyTorch model.
    pth_path = os.path.join(models_dir, "nnue.pth")
    torch.save(model.state_dict(), pth_path)

    # Save final quantized C++ model.
    bin_path = os.path.join(models_dir, "nnue.bin")
    export_quantized_model(model, bin_path)

    print(f"Saved final model: {pth_path}")
    print(f"Saved quantized model: {bin_path}")
    print(f"Saved training log: {log_path}")


if __name__ == "__main__":
    main()