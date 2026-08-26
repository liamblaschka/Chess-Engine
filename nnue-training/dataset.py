import pandas as pd
import torch
from torch.utils.data import Dataset

DATA_SIZE = 1_000_000

piece_index = {
    'p': 0, 'n': 1, 'b': 2, 'r': 3, 'q': 4, 'k': 5 
}

class PositionDataset(Dataset):
    def __init__(self, file_path):
        self.data = pd.read_csv(file_path)

        # Remove checkmate positions
        # self.data = self.data[
        #     ~self.data["Evaluation"].str.startswith("#")
        # ]
        
        # Remove shallow mates (remove mate in <= 5)
        mate_distance = self.data["Evaluation"].str.extract(r"#[-+]?(\d+)", expand=False)
        self.data = self.data[
            ~(mate_distance.notna() & (mate_distance.astype(float) <= 5))
        ]

        # Randomly select DATA_SIZE positions
        self.data = self.data.sample(n=DATA_SIZE, random_state=33).reset_index(drop=True)
    
    def __len__(self):
        return len(self.data)
    
    def __getitem__(self, i):
        row = self.data.iloc[i]
        
        fen = row["FEN"]
        evaluation = self.parse_evaluation(row["Evaluation"])
        
        features = self.get_features(fen)
        target = torch.tensor(evaluation, dtype=torch.float32)
        
        return features, target

    def get_features(self, fen):
        features = torch.zeros(768, dtype=torch.float32)
        
        rank = 7
        file = 0
        for c in fen:
            if c.isalpha():
                if c.isupper():
                    side = 0
                else:
                    side = 1
                
                square = rank * 8 + file
                i = side * 64 * 6 + piece_index[c.lower()] * 64 + square
                
                features[i] = 1.0
                
                file += 1
            elif c.isdigit():
                file += int(c)
            elif c == '/':
                rank -= 1
                file = 0
            elif c == ' ':
                break
        
        return features
    
    def parse_evaluation(self, evaluation):
        if evaluation.startswith("#+"):
            return 1000.0
        elif evaluation.startswith("#-"):
            return -1000.0
        else:
            return float(evaluation)
