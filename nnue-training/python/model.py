import torch
import torch.nn as nn

FEATURE_SIZE = 40960
A_SIZE = 64
H1_SIZE = 16
H2_SIZE = 8

class NNUE(nn.Module):
    def __init__(self, feature_size=FEATURE_SIZE, a_size=A_SIZE, h1_size=H1_SIZE, h2_size=H2_SIZE):
        super().__init__()
        
        self.accumulator_w = nn.Linear(feature_size, a_size)
        self.accumulator_b = nn.Linear(feature_size, a_size)
        self.h1 = nn.Linear(a_size * 2, h1_size)
        self.h2 = nn.Linear(h1_size, h2_size)
        self.output = nn.Linear(h2_size, 1)
    
    def forward(self, white_features, black_features, side_to_move):
        w = self.accumulator_w(white_features)
        b = self.accumulator_b(black_features)
        a_output = ((1 - side_to_move) * torch.cat([w, b], dim=1)) + (side_to_move * torch.cat([b, w], dim=1))
        a_output = torch.clamp(a_output, 0, 1)
        
        x = self.h1(a_output)
        x = torch.clamp(x, 0, 1)
        
        x = self.h2(x)
        x = torch.clamp(x, 0, 1)
        
        x = self.output(x)
        
        return x
