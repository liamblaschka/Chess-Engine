# NNUE Chess Engine

A C++ chess engine combining game-tree search with an Efficiently Updatable Neural Network (NNUE) for position evaluation. The model is trained in PyTorch and exported for inference in C++, integrating machine learning with legal move generation, search, and multithreading.

The engine has played against other bots and the games can be viewed on Lichess: [orange-bot](https://lichess.org/@/orange-bot).

## Highlights

- **Incremental neural evaluation:** Separate white and black accumulators cache feature-transformer outputs and update as moves are made and undone, reducing repeated computation during search.
- **Game-tree search:** Negamax with alpha-beta pruning, move ordering, and a Zobrist-keyed transposition table.
- **Parallel search:** Worker threads search root moves using separate game and evaluator states, with shared work coordination and result collection.
- **Chess implementation:** Legal move generation and reversible game state, including castling, en passant, and all four promotion choices.
- **Python-to-C++ integration:** A PyTorch training pipeline and a C++ evaluator connected through exported model weights.
- **UCI interface:** Command-based interaction through the Universal Chess Interface for use with compatible chess software.

## How It Works

For each position, the engine generates legal moves and searches the resulting game tree. Negamax expresses the search from the perspective of the side to move, while alpha-beta pruning skips branches that cannot improve the current result.

Move ordering helps the engine examine promising moves earlier. The transposition table stores search information under Zobrist position keys, allowing the engine to reuse results when different move sequences reach the same position.

At the search frontier, the NNUE estimates the position's value. Most moves change only a small subset of the input features, so the evaluator updates cached accumulators by adding and removing the contributions of changed features. This avoids rebuilding the feature-transformer output for every position. Accumulator state is maintained alongside the game state as the search makes and undoes moves.

Parallel search distributes root moves across worker threads. Each worker maintains its own mutable game and evaluator state, while shared coordination assigns work and collects results.

## Architecture

The implementation separates chess rules, search, and neural evaluation so that each component can be developed and checked independently.

| Component | Responsibility |
| --- | --- |
| Board and game state | Represent positions and apply or undo moves. |
| Move generation | Generate legal moves and handle special move rules. |
| Search | Explore positions using negamax, alpha-beta pruning, move ordering, and cached search results. |
| NNUE evaluator | Load model weights, maintain accumulators, and evaluate positions. |
| Training pipeline | Load labelled positions, train the PyTorch model, and export its weights. |
| UCI interface | Accept position and search commands and return the selected move. |

## NNUE Model

The evaluator uses separate feature transformers for the white and black perspectives. Their outputs are combined and passed through two hidden layers to produce a scalar position evaluation.

| Stage | Dimensions |
| --- | --- |
| White feature transformer | 40,960 → 512 |
| Black feature transformer | 40,960 → 512 |
| Combined accumulator outputs | 1,024 |
| First hidden layer | 1,024 → 32 |
| Second hidden layer | 32 → 32 |
| Output layer | 32 → 1 |

Training uses chess positions and evaluation labels from the [Chess Evaluations dataset](https://www.kaggle.com/datasets/ronakbadhe/chess-evaluations). The model learns to estimate position values from these labelled examples; the C++ search uses those estimates to select moves.

## Build and Run

### Requirements

- A C++ compiler compatible with the project's CMake configuration.
- CMake and a supported build tool, such as Make.
- NNUE weights compatible with the C++ model loader.
- Python, PyTorch, and the training pipeline's dependencies if retraining the model.

### Build the engine

Clone the repository and build from the project directory:

```bash
git clone https://github.com/liamblaschka/Chess-Engine.git
cd Chess-Engine

cmake -S . -B build
cmake --build build
```

### Run the engine

From the project directory:

```bash
./build/bin/chess-engine
```

Ensure the model weights are available at `model/nnue.bin`.

### UCI example

The executable accepts UCI commands through standard input. A basic session is:

```text
uci
isready
position startpos
go
```

The engine returns a `bestmove` response. Send `quit` to exit.

The engine can be used through a UCI compatible GUI.

## Train the NNUE

Training runs through `train.py` and uses a C++ dataloader built with its own `CMakeLists.txt`. Build this shared library before starting training.

### 1. Build the C++ dataloader
A C++17 compiler and CMake 3.16 or newer are required. From the directory containing the **training** `CMakeLists.txt`, run:

```bash
cmake -S . -B build
cmake --build build
```

This compiles `DataLoader.cpp`, `Dataset.cpp`, `SparseBatch.cpp`, and `SparseFeatures.cpp` into the `dataloader` shared library. The build places its output in `build/bin`.

### 2. Prepare the Python environment

From the directory containing `train.py`, create and activate a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

Install the training dependencies from `requirements.txt`:

```bash
python -m pip install -r requirements.txt
```

### 3. Prepare the dataset

Download the [Chess Evaluations dataset](https://www.kaggle.com/datasets/ronakbadhe/chess-evaluations) and place the extracted data files in `training/data/`, creating the directory if needed.

From the training directory, run the preprocessing script to split the data into training and validation sets:

```bash
python prepare_data.py
```

### 5. Use the trained model

Once training finishes, the model weights are exported to `model/nnue.bin`.

Run the engine using the instructions above to use the trained model.

If you modify the network or export format, ensure the Python exporter and C++ loader agree on layer dimensions, parameter ordering, numeric representation, and quantisation scales.


## Engineering Focus

This project brings together several areas of software and AI engineering:

- **State management:** Keeping board state, move history, and evaluator accumulators consistent through recursive search.
- **Algorithm design:** Combining pruning, move ordering, and transposition reuse to reduce redundant work.
- **Concurrency:** Coordinating parallel search while keeping each worker's mutable state separate.
- **ML integration:** Connecting a Python training pipeline to a C++ inference implementation.
- **Performance engineering:** Reusing neural feature computations across related positions.

## Project Status

This is an ongoing personal project, with development focused on search, evaluation, and engine performance. Games played by [orange-bot](https://lichess.org/@/orange-bot) provide examples of the engine in use.
