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
#include <algorithm>
#include <atomic>

Dataset::Dataset(const std::string& file_path, bool use_data_augmentation) : rng(33), next_batch_start(0), use_data_augmentation(use_data_augmentation) {
    readCSV(file_path);

    shuffled_indices.resize(data.size());
    std::iota(shuffled_indices.begin(), shuffled_indices.end(), 0);
    std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), rng);
}

void Dataset::readCSV(const std::string& file_path) {
    std::fstream fin;
    fin.open(file_path, std::ios::in);

    std::string line;
    std::getline(fin, line);
    
    while(std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string fen;
        std::string evaluation_str;
        std::getline(ss, fen, ',');
        std::getline(ss, evaluation_str);

        float evaluation = parseEvaluation(evaluation_str);

        if (use_data_augmentation && prob_distribution(rng) < 0.5) {
            flipFenPerspective(fen);
            evaluation = -evaluation;
        }

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

void Dataset::flipFenPerspective(std::string& fen) const {
    std::stringstream fen_ss(fen);

    std::string board;
    std::string turn;
    std::string castling;
    std::string en_passant;
    int halfmove;
    int fullmove;

    fen_ss >> board >> turn >> castling >> en_passant >> halfmove >> fullmove;

    std::stringstream board_ss(board);
    std::vector<std::string> ranks(8);

    // Reverse ranks
    for (int i = 7; i >= 0; i--) {
        std::getline(board_ss, ranks[i], '/');

        for (char& c : ranks[i]) {
            if (std::isupper(static_cast<unsigned char>(c))) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            } else if (std::islower(static_cast<unsigned char>(c))) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
        }
    }

    // Swap side to move
    if (turn == "w") {
        turn = "b";
    } else {
        turn = "w";
    }

    // Swap castling colours
    for (char& c : castling) {
        if (std::isupper(static_cast<unsigned char>(c))) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        } else if (std::islower(static_cast<unsigned char>(c))) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }

    // Flip en passant rank
    if (en_passant != "-") {
        en_passant[1] = static_cast<char>('9' - en_passant[1]);
    }

    // Rebuild FEN
    board = ranks[0];
    for (int i = 1; i < 8; i++) {
        board += "/" + ranks[i];
    }
    fen = board + " " + turn + " " + castling + " " + en_passant + " " + std::to_string(halfmove) + " " + std::to_string(fullmove);
}

float Dataset::parseEvaluation(const std::string& evaluation) const {
    if (evaluation[0] == '#') {
        if (evaluation[1] == '-') {
            return -3000;
        } else {
            return 3000;
        }
    }
    return std::stof(evaluation);
}

int Dataset::getNextBatchStart(int batch_size) {
    return next_batch_start.fetch_add(batch_size);
}

const TrainingEntry& Dataset::getEntry(int index) {
    return data[shuffled_indices[index]];
}

int Dataset::getDataSize() const { return data.size(); }

void Dataset::resetEpoch() {
    next_batch_start = 0;
    std::shuffle(shuffled_indices.begin(), shuffled_indices.end(), rng);
}


extern "C" {
    Dataset* Dataset_new(const char* file_path, bool use_data_augmentation) {
        return new Dataset(file_path, use_data_augmentation);
    }

    void Dataset_delete(Dataset* dataset) {
        delete dataset;
    }

    int Dataset_getDataSize(Dataset* dataset) {
        return dataset->getDataSize();
    }
}