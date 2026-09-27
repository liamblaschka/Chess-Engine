cmake_minimum_required(VERSION 3.16)

project(TrainingCPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(dataloader SHARED
    cpp/DataLoader.cpp
    cpp/Dataset.cpp
    cpp/SparseBatch.cpp
    cpp/SparseFeatures.cpp
)

target_include_directories(dataloader
    PUBLIC
        ${CMAKE_SOURCE_DIR}/cpp
)