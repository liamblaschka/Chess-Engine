#include "Board.h"
#include "Piece.h"
#include <array>
#include <iostream>

Board::Board() {
    squares = {};

    // White pieces
    squares[0] = {PieceType::Rook, Colour::White};
    squares[1] = {PieceType::Knight, Colour::White};
    squares[2] = {PieceType::Bishop, Colour::White};
    squares[3] = {PieceType::Queen, Colour::White};
    squares[4] = {PieceType::King, Colour::White};
    squares[5] = {PieceType::Bishop, Colour::White};
    squares[6] = {PieceType::Knight, Colour::White};
    squares[7] = {PieceType::Rook, Colour::White};

    for (int i = 8; i < 16; i++) {
        squares[i] = {PieceType::Pawn, Colour::White};
    }

    // Black pieces
    squares[56] = {PieceType::Rook, Colour::Black};
    squares[57] = {PieceType::Knight, Colour::Black};
    squares[58] = {PieceType::Bishop, Colour::Black};
    squares[59] = {PieceType::Queen, Colour::Black};
    squares[60] = {PieceType::King, Colour::Black};
    squares[61] = {PieceType::Bishop, Colour::Black};
    squares[62] = {PieceType::Knight, Colour::Black};
    squares[63] = {PieceType::Rook, Colour::Black};

    for (int i = 48; i < 56; i++) {
        squares[i] = {PieceType::Pawn, Colour::Black};
    }
}

void Board::makeMove(const Move& move) {
    switch (move.type) {
        case MoveType::Normal: {
            move_history.push_back({move, squares[move.to], move.to});

            squares[move.to] = squares[move.from];
            squares[move.from] = Piece();
            break;
        }
        case MoveType::EnPassant: {
            int from_rank = move.from / 8;
            int to_rank = move.to / 8;
            int direction = to_rank - from_rank;
            int captured_square = move.to - (8 * direction);
            move_history.push_back({move, squares[captured_square], captured_square});

            squares[move.to] = squares[move.from];
            squares[move.from] = Piece();
            squares[captured_square] = Piece();
            break;
        }
    }

    turn = oppositeColour(turn);
}

void Board::undoMove() {
    MoveState previous = move_history.back();
    move_history.pop_back();

    squares[previous.move.from] = squares[previous.move.to];
    squares[previous.move.to] = Piece();
    squares[previous.captured_square] = previous.captured_piece;

    turn = oppositeColour(turn);
}

const MoveState* Board::getLastMove() const {
    if (move_history.empty()) {
        return nullptr;
    }

    return &move_history.back();
}

bool Board::isKingInCheck(Colour colour) const {
    Colour opponent = oppositeColour(colour);

    // Find king
    int king_rank = -1;
    int king_file = -1;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            const Piece& piece = getPiece(rank, file);
            if (piece.colour == colour && piece.type == PieceType::King) {
                king_rank = rank;
                king_file = file;
                break;
            }
        }

        if (king_rank != -1) {
            break;
        }
    }

    // Pawn check
    int pawn_rank;
    if (opponent == Colour::White) {
        pawn_rank = king_rank - 1;
    } else {
        pawn_rank = king_rank + 1;
    }
    if (pawn_rank >= 0 && pawn_rank < 8) {
        if (king_file - 1 >= 0) {
            const Piece& attacker = getPiece(pawn_rank, king_file - 1);
            if (attacker.colour == opponent && attacker.type == PieceType::Pawn) {
                return true;
            }
        }
        if (king_file + 1 < 8) {
            const Piece& attacker = getPiece(pawn_rank, king_file + 1);
            if (attacker.colour == opponent && attacker.type == PieceType::Pawn) {
                return true;
            }
        }
    }

    // Knight check
    const int knight_positions[8][2] = {
        {king_rank + 2, king_file - 1},
        {king_rank + 2, king_file + 1},
        {king_rank + 1, king_file + 2},
        {king_rank - 1, king_file + 2},
        {king_rank - 2, king_file + 1},
        {king_rank - 2, king_file - 1},
        {king_rank - 1, king_file - 2},
        {king_rank + 1, king_file - 2}
    };
    for (const auto& position : knight_positions) {
        int knight_rank = position[0];
        int knight_file = position[1];

        if (knight_rank < 0 || knight_file < 0 || knight_rank >= 8 || knight_file >= 8) {
            continue;
        }

        const Piece& attacker = getPiece(knight_rank, knight_file);
        if (attacker.colour == opponent && attacker.type == PieceType::Knight) {
            return true;
        }
    }

    // Bishop, Queen check
    const int bishop_directions[4][2] = {
        {1, -1},
        {1, 1},
        {-1, 1},
        {-1, -1}
    };
    for (const auto& direction : bishop_directions) {
        int rank_direction = direction[0];
        int file_direction = direction[1];

        int attacker_rank = king_rank + rank_direction;
        int attacker_file = king_file + file_direction;
        while (attacker_rank >= 0 && attacker_file >= 0 && attacker_rank < 8 && attacker_file < 8) {
            const Piece& attacker = getPiece(attacker_rank, attacker_file);
            if (attacker.type != PieceType::None) {
                if (attacker.colour == opponent && (attacker.type == PieceType::Bishop || attacker.type == PieceType::Queen)) {
                    return true;
                }
                break;
            }

            attacker_rank += rank_direction;
            attacker_file += file_direction; 
        }
    }

    // Rook, Queen check
    const int rook_directions[4][2] = {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1}
    };
    for (const auto& direction : rook_directions) {
        int rank_direction = direction[0];
        int file_direction = direction[1];

        int attacker_rank = king_rank + rank_direction;
        int attacker_file = king_file + file_direction;
        while (attacker_rank >= 0 && attacker_file >= 0 && attacker_rank < 8 && attacker_file < 8) {
            const Piece& attacker = getPiece(attacker_rank, attacker_file);
            if (attacker.type != PieceType::None) {
                if (attacker.colour == opponent && (attacker.type == PieceType::Rook || attacker.type == PieceType::Queen)) {
                    return true;
                }
                break;
            }

            attacker_rank += rank_direction;
            attacker_file += file_direction; 
        }
    }

    // Opponent king check
    const int opponent_king_positions[8][2] = {
        {king_rank + 1, king_file},
        {king_rank, king_file + 1},
        {king_rank - 1, king_file},
        {king_rank, king_file - 1},
        {king_rank + 1, king_file - 1},
        {king_rank + 1, king_file + 1},
        {king_rank - 1, king_file + 1},
        {king_rank - 1, king_file - 1}
    };
    for (const auto& position : opponent_king_positions) {
        int opponent_king_rank = position[0];
        int opponent_king_file = position[1];

        if (opponent_king_rank < 0 || opponent_king_file < 0 || opponent_king_rank >= 8 || opponent_king_file >= 8) {
            continue;
        }

        const Piece& attacker = getPiece(opponent_king_rank, opponent_king_file);
        if (attacker.colour == opponent && attacker.type == PieceType::King) {
            return true;
        }
    }

    return false;
}

const Piece& Board::getPiece(int square) const { return squares[square]; }

const Piece& Board::getPiece(int rank, int file) const { return squares[rank * 8 + file]; }

void Board::setPiece(int rank, int file, Piece piece) { squares[rank * 8 + file] = piece; }

Colour Board::getTurn() const { return turn; }

void Board::setTurn(Colour colour) { turn = colour; }

void Board::clear() {
    squares.fill(Piece());
}

void Board::draw() {
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (8 - rank) << " ";
        for (int file = 0; file < 8; file++) {
            char piece_symbol;
            switch (squares[rank * 8 + file].type) {
                case PieceType::Pawn:
                    piece_symbol = 'P';
                    break;
                case PieceType::Rook:
                    piece_symbol = 'R';
                    break;
                case PieceType::Knight:
                    piece_symbol = 'N';
                    break;
                case PieceType::Bishop:
                    piece_symbol = 'B';
                    break;
                case PieceType::Queen:
                    piece_symbol = 'Q';
                    break;
                case PieceType::King:
                    piece_symbol = 'K';
                    break;
                case PieceType::None:
                    piece_symbol = '.';
                    break;
            }

            if (squares[rank * 8 + file].colour == Colour::Black) {
                piece_symbol -= ('A' - 'a');
            }

            std::cout << piece_symbol << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h" << std::endl;
}