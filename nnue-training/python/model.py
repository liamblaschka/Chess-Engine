import torch
import torch.nn as nn

HALF_KP_SIZE = 40960
HALF_RELATIVE_KP_SIZE = 2250
KING_FACTOR_SIZE = 64

A_SIZE = 256
H1_SIZE = 32
H2_SIZE = 16

class NNUE(nn.Module):
    def __init__(self, a_size=A_SIZE, h1_size=H1_SIZE, h2_size=H2_SIZE):
        super().__init__()
        
        self.w_half_kp = nn.Linear(HALF_KP_SIZE, a_size)
        self.b_half_kp = nn.Linear(HALF_KP_SIZE, a_size)
        
        self.w_half_relative_kp = nn.Linear(HALF_RELATIVE_KP_SIZE, a_size, bias=False)
        self.b_half_relative_kp = nn.Linear(HALF_RELATIVE_KP_SIZE, a_size, bias=False)
        
        self.w_king_factor = nn.Linear(KING_FACTOR_SIZE, a_size, bias=False)
        self.b_king_factor = nn.Linear(KING_FACTOR_SIZE, a_size, bias=False)
        
        self.h1 = nn.Linear(a_size * 2, h1_size)
        self.h2 = nn.Linear(h1_size, h2_size)
        self.output = nn.Linear(h2_size, 1)
    
    def forward(
        self,
        side_to_move: torch.Tensor,
        w_half_kp_feats: torch.Tensor, b_half_kp_feats: torch.Tensor,
        w_half_relative_kp_feats: torch.Tensor | None = None, b_half_relative_kp_feats: torch.Tensor | None = None,
        w_king_factor_feats: torch.Tensor | None = None, b_king_factor_feats: torch.Tensor | None = None
    ):
        w_out = self.w_half_kp(w_half_kp_feats)
        b_out = self.b_half_kp(b_half_kp_feats)
        
        if w_half_relative_kp_feats is not None:
            w_out = w_out + self.w_half_relative_kp(w_half_relative_kp_feats)
            b_out = b_out + self.b_half_relative_kp(b_half_relative_kp_feats)
            
            w_out = w_out + self.w_king_factor(w_king_factor_feats)
            b_out = b_out + self.b_king_factor(b_king_factor_feats)
        
        acc_out = ((1 - side_to_move) * torch.cat([w_out, b_out], dim=1)) + (side_to_move * torch.cat([b_out, w_out], dim=1))
        acc_out = torch.clamp(acc_out, 0, 1)
        
        x = self.h1(acc_out)
        x = torch.clamp(x, 0, 1)
        
        x = self.h2(x)
        x = torch.clamp(x, 0, 1)
        
        x = self.output(x)
        
        return x
    
    def coalesce_weights(self):
        with torch.no_grad():
            for half_kp_idx in range(HALF_KP_SIZE):
                piece_square = half_kp_idx % 64
                
                temp = half_kp_idx // 64
                p_idx = temp % 10
                king_square = temp // 10
                
                piece_rank = piece_square // 8
                piece_file = piece_square % 8
                king_rank = king_square // 8
                king_file = king_square % 8
                
                relative_file = piece_file - king_file + 7
                relative_rank = piece_rank - king_rank + 7
                
                relative_square = relative_rank * 15 + relative_file
                
                half_relative_kp_idx = (p_idx * 15 * 15) + relative_square
                k_idx = king_square
                
                self.w_half_kp.weight[:, half_kp_idx] += (
                    self.w_half_relative_kp.weight[:, half_relative_kp_idx] + self.w_king_factor.weight[:, k_idx]
                )
                self.b_half_kp.weight[:, half_kp_idx] += (
                    self.b_half_relative_kp.weight[:, half_relative_kp_idx] + self.b_king_factor.weight[:, k_idx]
                )
                
    def clip_weights(self):
        with torch.no_grad():
            self.h1.weight.clamp_(-128/64, 127/64)
            self.h2.weight.clamp_(-128/64, 127/64)
            self.output.weight.clamp_(-128/64, 127/64)