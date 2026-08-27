import pandas as pd
import torch
from torch.utils.data import Dataset

DATA_SIZE = 1_000_000

piece_index = {
    'p': 0, 'n': 1, 'b': 2, 'r': 3, 'q': 4
}

class PositionDataset(Dataset):
    def __init__(self, file_path):
        self.data = pd.read_csv(file_path)

        # Remove checkmate positions
        # self.data = self.data[
        #     ~self.data["Evaluation"].str.startswith("#")
        # ]
        
        # Remove shallow mates (remove mate in <= 5)
        # mate_distance = self.data["Evaluation"].str.extract(r"#[-+]?(\d+)", expand=False)
        # self.data = self.data[
        #     ~(mate_distance.notna() & (mate_distance.astype(float) <= 5))
        # ]

        # Randomly select DATA_SIZE positions
        self.data = self.data.sample(n=DATA_SIZE, random_state=33).reset_index(drop=True)
    
    def __len__(self):
        return len(self.data)
    
    def __getitem__(self, i):
        row = self.data.iloc[i]
        
        fen = row["FEN"]
        evaluation = self.parse_evaluation(row["Evaluation"])
        
        white_features, black_features, side_to_move = self.get_features(fen)
        target = torch.tensor(evaluation, dtype=torch.float32)
        side_to_move = torch.tensor([side_to_move], dtype=torch.float32)
        
        return white_features, black_features, target, side_to_move

    def get_features(self, fen):
        white_features = torch.zeros(40960, dtype=torch.float32)
        black_features = torch.zeros(40960, dtype=torch.float32)
        
        # Find kings
        rank = 7
        file = 0
        king_square = [-1, -1]
        for i in range(len(fen)):
            if king_square[0] != -1 and king_square[1] != -1:
                break
            
            if fen[i].isalpha():
                if fen[i] == 'K':
                    king_square[0] = rank * 8 + file
                elif fen[i] == 'k':
                    king_square[1] = rank * 8 + file
                
                file += 1
            elif fen[i].isdigit():
                file += int(fen[i])
            elif fen[i] == '/':
                rank -= 1
                file = 0
            elif fen[i] == ' ':
                break
        
        # Create features
        features = [white_features, black_features]
        for perspective in range(2):
            rank = 7
            file = 0
            for i in range(len(fen)):
                if fen[i].isalpha():
                    if fen[i] == 'K' or fen[i] == 'k':
                        file += 1
                        continue
                    
                    if fen[i].isupper():
                        side = 0
                    else:
                        side = 1
                    
                    square = rank * 8 + file
                    p_idx = piece_index[fen[i].lower()] * 2 + side
                    halfkp_idx = square + (p_idx + king_square[perspective] * 10) * 64
                    
                    features[perspective][halfkp_idx] = 1.0
                    
                    file += 1
                elif fen[i].isdigit():
                    file += int(fen[i])
                elif fen[i] == '/':
                    rank -= 1
                    file = 0
                elif fen[i] == ' ':
                    break
        
        side_to_move = fen.split()[1]
        if side_to_move == 'w':
            side_to_move = 0
        else:
            side_to_move = 1
        
        return white_features, black_features, side_to_move
    
    def parse_evaluation(self, evaluation):
        if evaluation.startswith("#+"):
            return 1000.0
        elif evaluation.startswith("#-"):
            return -1000.0
        else:
            return float(evaluation)
