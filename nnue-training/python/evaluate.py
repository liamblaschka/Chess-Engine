import torch

from model import NNUE
from dataset import PositionDataset

model = NNUE()
model.load_state_dict(
    torch.load("nnue.pth", map_location="cpu")
)
model.eval()

dataset = PositionDataset("train_data/chessData.csv")

fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

white_features, black_features, stm = dataset.get_features(fen)

white_features = white_features.unsqueeze(0)
black_features = black_features.unsqueeze(0)
side_to_move = torch.tensor([[stm]], dtype=torch.float32)

with torch.no_grad():
    w = model.accumulator_w(white_features)
    b = model.accumulator_b(black_features)

    if stm == 0:
        a = torch.cat([w, b], dim=1)
    else:
        a = torch.cat([b, w], dim=1)

    a = torch.clamp(a, 0, 1)

    h1_raw = model.h1(a)
    h1 = torch.clamp(h1_raw, 0, 1)

    h2_raw = model.h2(h1)
    h2 = torch.clamp(h2_raw, 0, 1)

    output = model.output(h2)

print("Accumulator:")
print(a)

print("\nH1 raw:")
print(h1_raw)

print("\nH1:")
print(h1)

print("\nH2 raw:")
print(h2_raw)

print("\nH2:")
print(h2)

print("\nOutput:")
print(output.item())

print("\nOutput bias:")
print(model.output.bias.item())