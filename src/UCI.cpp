#include "UCI.h"
#include "Board.h"
#include "Move.h"
#include "Piece.h"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

void UCI::run() {
    std::string command;

    while (std::getline(std::cin, command)) {
        handleCommand(command);
    }
}

void UCI::handleCommand(const std::string& command) {
    if (command == "uci") {
        uci();
    } else if (command == "isready") {
        isReady();
    } else if (command == "ucinewgame") {
        newGame();
    } else if (command.rfind("position", 0) == 0) {
        position(command);
    } else if (command.rfind("go", 0) == 0) {
        go();
    } else if (command == "quit") {
        std::exit(0);
    }
}

void UCI::uci() {
    std::cout << "id name MyChessEngine\n";
    std::cout << "id author Liam\n";
    std::cout << "uciok\n";
    std::cout.flush();
}

void UCI::isReady() {
    std::cout << "readyok\n";
    std::cout.flush();
}

void UCI::newGame() {
    game = Game();
}

void UCI::position(const std::string& command) {
    std::istringstream stream(command);

    std::string token;

    // Skip "position"
    stream >> token;

    if (!(stream >> token)) {
        throw std::invalid_argument("Invalid position command");
    }

    if (token == "startpos") {
        game = Game();
    } else if (token == "fen") {
        std::string fen;
        std::string field;
        for (int i = 0; i < 6; i++) {
            if (!(stream >> field)) {
                throw std::invalid_argument("Invalid FEN");
            }

            if (i > 0) {
                fen += ' ';
            }

            fen += field;
        }

        game.setPosition(fen);
    } else {
        throw std::invalid_argument(
            "Invalid position command"
        );
    }

    // Apply moves after the position
    while (stream >> token) {
        if (token == "moves") {
            continue;
        }
        Move move = parseMove(token);
        game.makeMove(move);
    }
}

void UCI::go() {
    Move best_move = search.minimax(game).first;

    std::cout << "bestmove " << moveToUCI(best_move) << '\n';
    std::cout.flush();
}

int UCI::squareFromUCI(const std::string& square) const {
    if (square.length() != 2) {
        throw std::invalid_argument(
            "Invalid square: " + square
        );
    }

    char file = square[0];
    char rank = square[1];

    if (file < 'a' || file > 'h' ||
        rank < '1' || rank > '8') {
        throw std::invalid_argument(
            "Invalid square: " + square
        );
    }

    int file_index = file - 'a';
    int rank_index = rank - '1';

    return rank_index * 8 + file_index;
}

std::string UCI::squareToUCI(int square) const {
    int rank = square / 8;
    int file = square % 8;

    std::string result;

    result += static_cast<char>('a' + file);
    result += static_cast<char>('1' + rank);

    return result;
}

Move UCI::parseMove(const std::string& move_string) {
    if (move_string.length() != 4 &&
        move_string.length() != 5) {
        throw std::invalid_argument(
            "Invalid UCI move: " + move_string
        );
    }

    int from = squareFromUCI(move_string.substr(0, 2));
    int to = squareFromUCI(move_string.substr(2, 2));

    const std::vector<Move>& legal_moves = game.getLegalMoves();

    for (const Move& move : legal_moves) {
        if (move.from != from ||
            move.to != to) {
            continue;
        }

        // Normal move, capture, castling, or en passant
        if (move_string.length() == 4) {
            if (move.type != MoveType::Promotion) {
                return move;
            }
            continue;
        }

        // Promotion
        char promotion = move_string[4];
        PieceType promotion_type;
        switch (promotion) {
            case 'q':
                promotion_type = PieceType::Queen;
                break;
            case 'r':
                promotion_type = PieceType::Rook;
                break;
            case 'b':
                promotion_type = PieceType::Bishop;
                break;
            case 'n':
                promotion_type = PieceType::Knight;
                break;
            default:
                throw std::invalid_argument(
                    "Invalid promotion: " + move_string
                );
        }

        if (move.type == MoveType::Promotion &&
            move.promotion_piece.type == promotion_type) {
            return move;
        }
    }

    throw std::invalid_argument(
        "Illegal move: " + move_string
    );
}

std::string UCI::moveToUCI(const Move& move) const {
    std::string result;

    result += squareToUCI(move.from);
    result += squareToUCI(move.to);

    if (move.type == MoveType::Promotion) {
        switch (move.promotion_piece.type) {
            case PieceType::Queen:
                result += 'q';
                break;

            case PieceType::Rook:
                result += 'r';
                break;

            case PieceType::Bishop:
                result += 'b';
                break;

            case PieceType::Knight:
                result += 'n';
                break;

            default:
                break;
        }
    }

    return result;
}