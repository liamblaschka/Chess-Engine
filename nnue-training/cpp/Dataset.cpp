#include "Dataset.h"
#include "TrainingEntry.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdint>
#include <cctype>
#include <random>
#include <numeric>

Dataset::Dataset(const std::string& file_path, int data_size) : shuffled_index(0), rng(33) {
    shuffled_indices.resize(data_size);
    std::iota(shuffled_indices.begin(), shuffled_indices.end(), 0);
    resetEpoch();

    readCSV(file_path, data_size);
}

void Dataset::readCSV(const std::string& file_path, int data_size) {
    std::fstream fin;
    fin.open(file_path, std::ios::in);

    std::string line;
    std::getline(fin, line);
    
    for (int i = 0; i < data_size; i++) {
        std::getline(fin, line);

        std::stringstream ss(line);
        std::string fen;
        std::string evaluation_str;
        std::getline(ss, fen, ',');
        std::getline(ss, evaluation_str);

        float evaluation = parseEvaluation(evaluation_str);

        std::vector<Piece> pieces;
        std::uint8_t white_king_square = 0;
        std::uint8_t black_king_square = 0;
        Side side_to_move = Side::White;

        int rank = 7;
        int file = 0;
        for (int j = 0; j < fen.length(); j++) {
            std::uint8_t square = rank * 8 + file;

            if (fen[j] == 'K') {
                white_king_square = square;
                file++;
                continue;
            } else if (fen[j] == 'k') {
                black_king_square = square;
                file++;
                continue;
            }

            if (std::isalpha(fen[j])) {
                PieceType type;
                Side side;
                if (std::isupper(fen[j])) {
                    side = Side::White;
                } else {
                    side = Side::Black;
                }
                switch (std::toupper(fen[j])) {
                    case 'P':
                        type = PieceType::Pawn;
                        break;
                    case 'N':
                        type = PieceType::Knight;
                        break;
                    case 'B':
                        type = PieceType::Bishop;
                        break;
                    case 'R':
                        type = PieceType::Rook;
                        break;
                    case 'Q':
                        type = PieceType::Queen;
                        break;
                }

                pieces.push_back({type, side, square});

                file++;
            } else if (std::isdigit(fen[j])) {
                file += fen[j] - '0';
            } else if (fen[j] == '/') {
                rank -= 1;
                file = 0;
            } else if (fen[j] == ' ') {
                if (fen[j + 1] == 'w') {
                    side_to_move = Side::White;
                } else {
                    side_to_move = Side::Black;
                }
                break;
            }
        }

        data.push_back({pieces, white_king_square, black_king_square, side_to_move, evaluation});
    }
}

float Dataset::parseEvaluation(const std::string& evaluation) const {
    if (evaluation[0] == '#') {
        if (evaluation[1] == '-') {
            return -1000;
        } else {
            return 1000;
        }
    }
    return std::stof(evaluation);
}

const TrainingEntry& Dataset::getNextEntry() {
    int index = shuffled_indices[shuffled_index++];

    return data[index];
}

void Dataset::resetEpoch() {
    shuffled_index = 0;
    std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), rng);
}


extern "C" {
    Dataset* Dataset_new(const char* file_path, int data_size) {
        return new Dataset(file_path, data_size);
    }

    void Dataset_delete(Dataset* dataset) {
        delete dataset;
    }
}