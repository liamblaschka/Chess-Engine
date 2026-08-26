import torch
import torch.nn as nn

FEATURE_SIZE = 768
A_SIZE = 128
H1_SIZE = 16

class NNUE(nn.Module):
    def __init__(self, feature_size=FEATURE_SIZE, a_size=A_SIZE, h1_size=H1_SIZE):
        super().__init__()
        
        self.accumulator = nn.Linear(feature_size, a_size)
        self.h1 = nn.Linear(a_size, h1_size)
        self.output = nn.Linear(h1_size, 1)
    
    def forward(self, features):
        x = self.accumulator(features)
        x = torch.clamp(x, 0, 1)
        
        x = self.h1(x)
        x = torch.clamp(x, 0, 1)
        
        x = self.output(x)
        
        return x
