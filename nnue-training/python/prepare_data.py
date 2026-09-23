import pandas as pd
from pathlib import Path

SEED = 33

TRAIN_RATIO = 0.95
VALIDATION_RATIO = 0.05

NORMAL_P = 0.9
RANDOM_P = 0.05
TACTIC_P = 0.05

DATA_DIR = Path("data")

# Chess position and evaluation data sourced from: https://www.kaggle.com/datasets/ronakbadhe/chess-evaluations/data
NORMAL_PATH = DATA_DIR / "chessData.csv"
RANDOM_PATH = DATA_DIR / "random_evals.csv"
TACTIC_PATH = DATA_DIR / "tactic_evals.csv"

TRAIN_PATH = DATA_DIR / "train.csv"
VALIDATION_PATH = DATA_DIR / "validation.csv"


def load_dataset(path):
    data = pd.read_csv(path)
    
    data = data[["FEN", "Evaluation"]] # Using only FEN and Evaluation, excluding best move from tactics_evals.csv
    data = data.dropna()
    
    return data

def split_dataset(data):
    data = data.sample(frac=1, random_state = SEED).reset_index(drop=True)
    
    split_index = int(len(data) * TRAIN_RATIO)
    
    train = data.iloc[:split_index]
    validation = data.iloc[split_index:]
    
    return train, validation

def sample_mixture(normal, random, tactic):
    max_size = min(len(normal) / NORMAL_P, len(random) / RANDOM_P, len(tactic) / TACTIC_P)
    
    normal_count = int(max_size * NORMAL_P)
    random_count = int(max_size * RANDOM_P)
    tactic_count = int(max_size * TACTIC_P)
    
    normal = normal.sample(n=normal_count, random_state=SEED)
    random = random.sample(n=random_count, random_state=SEED)
    tactic = tactic.sample(n=random_count, random_state=SEED)
    
    combined = pd.concat([normal, random, tactic], ignore_index=True)
    
    return combined.sample(frac=1, random_state=SEED).reset_index(drop=True)


def main():
    print("Loading datasets...")
    
    normal = load_dataset(NORMAL_PATH)
    random = load_dataset(RANDOM_PATH)
    tactic = load_dataset(TACTIC_PATH)
    
    print(f"Normal: {len(normal):,}")
    print(f"Random: {len(random):,}")
    print(f"Tactic: {len(tactic):,}")
    
    normal_train, normal_validation = split_dataset(normal)
    random_train, random_validation = split_dataset(random)
    tactic_train, tactic_validation = split_dataset(tactic)
    
    print("Creating training mixture...")
    train = sample_mixture(normal_train, random_train, tactic_train)
    
    print("Creating validation mixture...")
    validation = sample_mixture(normal_validation, random_validation, tactic_validation)
    
    print(f"Training positions: {len(train):,}")
    print(f"Validation positions: {len(validation):,}")
    
    train.to_csv(TRAIN_PATH, index=False)
    validation.to_csv(VALIDATION_PATH, index=False)
    
    print(f"Saved {TRAIN_PATH}")
    print(f"Saved {VALIDATION_PATH}")
    
    
if __name__ == "__main__":
    main()
