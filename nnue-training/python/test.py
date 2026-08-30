import torch
import numpy as np
import os

from model import NNUE

model = NNUE()

model.load_state_dict(
    torch.load("nnue.pth", map_location="cpu")
)

model.eval()

with open("nnue.bin", "wb") as f:
    for layer in [
        model.accumulator_w,
        model.accumulator_b,
        model.h1,
        model.h2,
        model.output
    ]:
        weights = np.ascontiguousarray(
            layer.weight
            .detach()
            .cpu()
            .numpy()
            .astype(np.float32)
            .T
        )

        biases = np.ascontiguousarray(
            layer.bias
            .detach()
            .cpu()
            .numpy()
            .astype(np.float32)
        )

        f.write(weights.tobytes())
        f.write(biases.tobytes())


print("nnue.bin size:", os.path.getsize("nnue.bin"))

expected_size = 20_980_868

assert os.path.getsize("nnue.bin") == expected_size, (
    f"Wrong nnue.bin size! "
    f"Expected {expected_size}, "
    f"got {os.path.getsize('nnue.bin')}"
)