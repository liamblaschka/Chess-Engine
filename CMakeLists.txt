cmake_minimum_required(VERSION 3.16)

project(ChessEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

add_executable(chess-engine
    src/main.cpp
    src/UCI.cpp
    src/MoveGenerator.cpp
    src/Piece.cpp
    src/Board.cpp
    src/Game.cpp
    src/Search.cpp
    src/TranspositionTable.cpp
    src/NNUE.cpp
)

target_compile_options(chess-engine PRIVATE
    -O3
    -march=native
)

add_executable(datagenerator
    src/DataGeneratorMain.cpp
    src/DataGenerator.cpp
    src/MoveGenerator.cpp
    src/Piece.cpp
    src/Board.cpp
    src/Game.cpp
    src/Search.cpp
    src/TranspositionTable.cpp
    src/NNUE.cpp
)

target_compile_options(datagenerator PRIVATE
    -O3
    -march=native
)