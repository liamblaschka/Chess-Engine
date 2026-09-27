# Chess-Engine

An Efficiently Updatable Neural Network (NNUE) chess engine in C++. The NNUE model is trained in PyTorch with its weights exported to C++, which includes the game-tree search and game implementation.

The bot has played games against other bots on Lichess as <a href="https://lichess.org/@/orange-bot">orange-bot</a>.

## Highlights
- **NNUE Evaluation:** NNUE model trained with PyTorch and loaded into C++. Separate accumulators for each side store the feature transformer outputs and support incremental feature updates as moves are made and undone. The model is trained on chess position and evaluation data sourced from: https://www.kaggle.com/datasets/ronakbadhe/chess-evaluations.
- **Search:** Negamax with alpha-beta pruning, move ordering, transposition table, and parallel search.
- **Concurrency:** Worker threads evaluate root moves using separate game and evaluator states, with shared work coordination to facilitate parallel search.

## How it works
The engine generates legal moves, searches the resulting positions, and uses the NNUE to evaluate positions at the search frontier. Alpha-beta pruning cuts off branches that cannot improve the result; move ordering and the transposition table help the search spend its time on promising positions. The evaluator updates its accumulators for changed piece-square features instead of rebuilding every input after each move.

The code separates board state, game rules, move generation, search, and neural evaluation. This made it possible to validate chess rules independently and add search and evaluation optimisations in stages.

## Technology
C++, Python, and PyTorch.

## Instructions
- To compile create a build directory and within run `cmake ..`, then run `make`.
- From the bin directory run `chess` executable.
- The engine uses UCI (Universal Chess Interface). 
