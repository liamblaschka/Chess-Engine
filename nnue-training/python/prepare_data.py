import pandas as pd
from pathlib import Path

SEED = 33

TRAIN_RATIO = 0.95
VALIDATION_RATIO = 0.05

NORMAL_P = 0.925
RANDOM_P = 0.075
TACTIC_P = 0.0

DATA_DIR = Path("data")

# Chess position and evaluation data sourced from: https://www.kaggle.com/datasets/ronakbadhe/chess-evaluations/data
NORMAL_PATH = DATA_DIR / "chessData.csv"
RANDOM_PATH = DATA_DIR / "random_evals.csv"
TACTIC_PATH = DATA_DIR / "tactic_evals.csv"

TRAIN_PATH = DATA_DIR / "train.csv"
VALIDATION_PATH = DATA_DIR / "validation.csv"


def load_dataset(path):
    data = pd.read_csv(path)
    
    data = data[["FEN", "Evaluation"]]
    data = data.dropna()
    
    return data

def split_dataset(data):
    data = data.sample(frac=1, random_state = SEED).reset_index(drop=True)
    
    split_index = int(len(data) * TRAIN_RATIO)
    
    train = data.iloc[:split_index]
    validation = data.iloc[split_index:]
    
    return train, validation

def sample_mixture(normal, random, tactic):
    possible_sizes = []
    if NORMAL_P > 0:
        possible_sizes.append(len(normal) / NORMAL_P)
    if RANDOM_P > 0:
        possible_sizes.append(len(random) / RANDOM_P)
    if TACTIC_P > 0:
        possible_sizes.append(len(tactic) / TACTIC_P)
    max_size = min(possible_sizes)

    samples = []
    
    normal_count, random_count, tactic_count = int(max_size * NORMAL_P), int(max_size * RANDOM_P), int(max_size * TACTIC_P)

    if normal_count > 0:
        samples.append(normal.sample(n=normal_count, random_state=SEED))

    if random_count > 0:
        samples.append(random.sample(n=random_count, random_state=SEED))

    if tactic_count > 0:
        samples.append(tactic.sample(n=tactic_count, random_state=SEED))
        
    print(f"Normal: {normal_count:,}, Random: {random_count:,}, Tactic: {tactic_count:,}")

    combined = pd.concat(samples, ignore_index=True)

    return combined.sample(frac=1, random_state=SEED).reset_index(drop=True)


def main():
    print("Loading datasets...")

    normal = load_dataset(NORMAL_PATH)
    random = load_dataset(RANDOM_PATH)
    tactic = load_dataset(TACTIC_PATH)

    print(f"Normal: {len(normal):,}")
    print(f"Random: {len(random):,}")
    print(f"Tactic: {len(tactic):,}")

    print("Creating mixture...")
    data = sample_mixture(normal, random, tactic)
    
    print("Splitting training and validation data...")
    train, validation = split_dataset(data)

    print(f"Training positions: {len(train):,}")
    print(f"Validation positions: {len(validation):,}")

    train.to_csv(TRAIN_PATH, index=False)
    validation.to_csv(VALIDATION_PATH, index=False)

    print(f"Saved {TRAIN_PATH}")
    print(f"Saved {VALIDATION_PATH}")
    
    
if __name__ == "__main__":
    main()
