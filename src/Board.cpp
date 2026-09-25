#include "Board.h"
#include "Piece.h"
#include "SquareConstants.h"
#include "Zobrist.hpp"
#include <array>
#include <cmath>

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

    turn = Colour::White;

    white_king_square = Square::E1;
    black_king_square = Square::E8;

    white_castle_rights = {true, true};
    black_castle_rights = {true, true};

    Zobrist::initialise();
    zobrist_key = calculateZobristKey();
}

void Board::makeMove(const Move& move) {
    Piece moving_piece = squares[move.from];

    int previous_en_passant_square = en_passant_square;

    // Move
    switch (move.type) {
        case MoveType::Normal: {
            move_history.push_back({move, squares[move.to], move.to, white_castle_rights, black_castle_rights, previous_en_passant_square, zobrist_key});

            // Castle rights if rook is captured
            const Piece& captured_piece = squares[move.to];
            if (captured_piece.type == PieceType::Rook) {
                if (captured_piece.colour == Colour::White) {
                    if (move.to == Square::A1 && white_castle_rights.queen_side) {
                        white_castle_rights.queen_side = false;
                        zobrist_key ^= Zobrist::castle_rights[0];
                    } else if (move.to == Square::H1 && white_castle_rights.king_side) {
                        white_castle_rights.king_side = false;
                        zobrist_key ^= Zobrist::castle_rights[1];
                    }
                } else if (captured_piece.colour == Colour::Black) {
                    if (move.to == Square::A8 && black_castle_rights.queen_side) {
                        black_castle_rights.queen_side = false;
                        zobrist_key ^= Zobrist::castle_rights[2];
                    } else if (move.to == Square::H8 && black_castle_rights.king_side) {
                        black_castle_rights.king_side = false;
                        zobrist_key ^= Zobrist::castle_rights[3];
                    }
                }
            }

            setPiece(move.to, squares[move.from]);
            setPiece(move.from, Piece());
            break;
        }
        case MoveType::Castle: {
            move_history.push_back({move, squares[move.to], move.to, white_castle_rights, black_castle_rights, previous_en_passant_square, zobrist_key});

            setPiece(move.to, squares[move.from]);
            setPiece(move.from, Piece());

            if (move.to == Square::C1) {
                // White Queen-side
                setPiece(Square::D1, squares[Square::A1]);
                setPiece(Square::A1, Piece());
            } else if (move.to == Square::G1) {
                // White King-side
                setPiece(Square::F1, squares[Square::H1]);
                setPiece(Square::H1, Piece());
            } else if (move.to == Square::C8) {
                // Black Queen-side
                setPiece(Square::D8, squares[Square::A8]);
                setPiece(Square::A8, Piece());
            } else {
                // Black King-side
                setPiece(Square::F8, squares[Square::H8]);
                setPiece(Square::H8, Piece());
            }
            break;
        }
        case MoveType::EnPassant: {
            int from_rank = move.from / 8;
            int to_rank = move.to / 8;
            int direction = to_rank - from_rank;
            int captured_square = move.to - (8 * direction);
            move_history.push_back({move, squares[captured_square], captured_square, white_castle_rights, black_castle_rights, previous_en_passant_square, zobrist_key});

            setPiece(move.to, squares[move.from]);
            setPiece(move.from, Piece());
            setPiece(captured_square, Piece());
            break;
        }
        case MoveType::Promotion: {
            move_history.push_back({move, squares[move.to], move.to, white_castle_rights, black_castle_rights, previous_en_passant_square, zobrist_key});

            setPiece(move.to, move.promotion_piece);
            setPiece(move.from, Piece());
            break;
        }
    }

    // Update castling rights, king square
    const Piece& piece = squares[move.to];
    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            if (white_castle_rights.queen_side) {
                white_castle_rights.queen_side = false;
                zobrist_key ^= Zobrist::castle_rights[0];
            }
            if (white_castle_rights.king_side) {
                white_castle_rights.king_side = false;
                zobrist_key ^= Zobrist::castle_rights[1];
            }
        } else {
            if (black_castle_rights.queen_side) {
                black_castle_rights.queen_side = false;
                zobrist_key ^= Zobrist::castle_rights[2];
            }
            if (black_castle_rights.king_side) {
                black_castle_rights.king_side = false;
                zobrist_key ^= Zobrist::castle_rights[3];
            }
        }
    } else if (piece.type == PieceType::Rook) {
        if (piece.colour == Colour::White && (move.from == Square::A1 || move.from == Square::H1)) {
            if (move.from == Square::A1 && white_castle_rights.queen_side) {
                white_castle_rights.queen_side = false;
                zobrist_key ^= Zobrist::castle_rights[0];
            } else if (move.from == Square::H1 && white_castle_rights.king_side) {
                white_castle_rights.king_side = false;
                zobrist_key ^= Zobrist::castle_rights[1];
            }
        } else if (move.from == Square::A8 || move.from == Square::H8) {
            if (move.from == Square::A8 && black_castle_rights.queen_side) {
                black_castle_rights.queen_side = false;
                zobrist_key ^= Zobrist::castle_rights[2];
            } else if (move.from == Square::H8 && black_castle_rights.king_side) {
                black_castle_rights.king_side = false;
                zobrist_key ^= Zobrist::castle_rights[3];
            }
        }
    }

    // Update en passant state
    if (en_passant_square != -1) {
        zobrist_key ^= Zobrist::en_passant[en_passant_square % 8];
    }
    en_passant_square = -1;
    if (moving_piece.type == PieceType::Pawn && std::abs(move.to - move.from) == 16) {
        en_passant_square = (move.from + move.to) / 2;
        zobrist_key ^= Zobrist::en_passant[en_passant_square % 8];
    }

    turn = oppositeColour(turn);
    zobrist_key ^= Zobrist::side_to_move;
}

void Board::undoMove() {
    MoveState previous = move_history.back();
    move_history.pop_back();

    switch (previous.move.type) {
        case MoveType::Normal: {
            squares[previous.move.from] = squares[previous.move.to];
            squares[previous.move.to] = previous.captured_piece;
            break;
        }
        case MoveType::Castle: {
            if (previous.move.to == Square::C1) {
                // White Queen-side
                squares[Square::A1] = squares[Square::D1];
                squares[Square::D1] = Piece();
            } else if (previous.move.to == Square::G1) {
                // White King-side
                squares[Square::H1] = squares[Square::F1];
                squares[Square::F1] = Piece();
            } else if (previous.move.to == Square::C8) {
                // Black Queen-side
                squares[Square::A8] = squares[Square::D8];
                squares[Square::D8] = Piece();
            } else {
                // Black King-side
                squares[Square::H8] = squares[Square::F8];
                squares[Square::F8] = Piece();
            }

            squares[previous.move.from] = squares[previous.move.to];
            squares[previous.move.to] = Piece();

            break;
        }
        case MoveType::EnPassant: {
            squares[previous.move.from] = squares[previous.move.to];
            squares[previous.move.to] = Piece();
            squares[previous.captured_square] = previous.captured_piece;
            break;
        }
        case MoveType::Promotion: {
            squares[previous.move.from] = Piece(PieceType::Pawn, previous.move.promotion_piece.colour);
            squares[previous.move.to] = previous.captured_piece;
            break;
        }
    }

    const Piece& piece = squares[previous.move.from];
     if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            white_king_square = previous.move.from;
        } else {
            black_king_square = previous.move.from;
        }
    }
            
    white_castle_rights = previous.white_castle_rights;
    black_castle_rights = previous.black_castle_rights;
    en_passant_square = previous.en_passant_square;

    turn = oppositeColour(turn);

    zobrist_key = previous.zobrist_key;
}

const MoveState* Board::getLastMove() const {
    if (move_history.empty()) {
        return nullptr;
    }

    return &move_history.back();
}

bool Board::isSquareAttacked(int rank, int file, Colour attacking_colour) const {
    // Pawn attack
    int pawn_rank;
    if (attacking_colour == Colour::White) {
        pawn_rank = rank - 1;
    } else {
        pawn_rank = rank + 1;
    }
    if (pawn_rank >= 0 && pawn_rank < 8) {
        if (file - 1 >= 0) {
            const Piece& attacker = getPiece(pawn_rank, file - 1);
            if (attacker.colour == attacking_colour && attacker.type == PieceType::Pawn) {
                return true;
            }
        }
        if (file + 1 < 8) {
            const Piece& attacker = getPiece(pawn_rank, file + 1);
            if (attacker.colour == attacking_colour && attacker.type == PieceType::Pawn) {
                return true;
            }
        }
    }

    // Knight attack
    const int knight_positions[8][2] = {
        {rank + 2, file - 1},
        {rank + 2, file + 1},
        {rank + 1, file + 2},
        {rank - 1, file + 2},
        {rank - 2, file + 1},
        {rank - 2, file - 1},
        {rank - 1, file - 2},
        {rank + 1, file - 2}
    };
    for (const auto& position : knight_positions) {
        int knight_rank = position[0];
        int knight_file = position[1];

        if (knight_rank < 0 || knight_file < 0 || knight_rank >= 8 || knight_file >= 8) {
            continue;
        }

        const Piece& attacker = getPiece(knight_rank, knight_file);
        if (attacker.colour == attacking_colour && attacker.type == PieceType::Knight) {
            return true;
        }
    }

    // Bishop, Queen attack
    const int bishop_directions[4][2] = {
        {1, -1},
        {1, 1},
        {-1, 1},
        {-1, -1}
    };
    for (const auto& direction : bishop_directions) {
        int rank_direction = direction[0];
        int file_direction = direction[1];

        int attacker_rank = rank + rank_direction;
        int attacker_file = file + file_direction;
        while (attacker_rank >= 0 && attacker_file >= 0 && attacker_rank < 8 && attacker_file < 8) {
            const Piece& attacker = getPiece(attacker_rank, attacker_file);
            if (attacker.type != PieceType::None) {
                if (attacker.colour == attacking_colour && (attacker.type == PieceType::Bishop || attacker.type == PieceType::Queen)) {
                    return true;
                }
                break;
            }

            attacker_rank += rank_direction;
            attacker_file += file_direction; 
        }
    }

    // Rook, Queen attack
    const int rook_directions[4][2] = {
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1}
    };
    for (const auto& direction : rook_directions) {
        int rank_direction = direction[0];
        int file_direction = direction[1];

        int attacker_rank = rank + rank_direction;
        int attacker_file = file + file_direction;
        while (attacker_rank >= 0 && attacker_file >= 0 && attacker_rank < 8 && attacker_file < 8) {
            const Piece& attacker = getPiece(attacker_rank, attacker_file);
            if (attacker.type != PieceType::None) {
                if (attacker.colour == attacking_colour && (attacker.type == PieceType::Rook || attacker.type == PieceType::Queen)) {
                    return true;
                }
                break;
            }

            attacker_rank += rank_direction;
            attacker_file += file_direction; 
        }
    }

    // King attack
    const int king_positions[8][2] = {
        {rank + 1, file},
        {rank, file + 1},
        {rank - 1, file},
        {rank, file - 1},
        {rank + 1, file - 1},
        {rank + 1, file + 1},
        {rank - 1, file + 1},
        {rank - 1, file - 1}
    };
    for (const auto& position : king_positions) {
        int king_rank = position[0];
        int king_file = position[1];

        if (king_rank < 0 || king_file < 0 || king_rank >= 8 || king_file >= 8) {
            continue;
        }

        const Piece& attacker = getPiece(king_rank, king_file);
        if (attacker.colour == attacking_colour && attacker.type == PieceType::King) {
            return true;
        }
    }

    return false;
}

bool Board::isSquareAttacked(int square, Colour attacking_colour) const {
    int rank = square / 8;
    int file = square % 8;

    return isSquareAttacked(rank, file, attacking_colour);
}

int Board::getKingSquare(Colour colour) const {
    if (colour == Colour::White) {
        return white_king_square;
    } else {
        return black_king_square;
    }
}

bool Board::isKingInCheck(Colour colour) const {
    int king_square = getKingSquare(colour);

    return isSquareAttacked(king_square, oppositeColour(colour));
}

// Insufficient material rules as per: https://support.chess.com/en/articles/8705277-what-does-insufficient-mating-material-mean
bool Board::isInsufficientMaterial() const {
    int white_bishops = 0;
    int black_bishops = 0;
    int white_knights = 0;
    int black_knights = 0;

    for (const auto& piece : squares) {
        if (piece.type == PieceType::None || piece.type == PieceType::King) {
            continue;
        }

        if (piece.type == PieceType::Bishop) {
            if (piece.colour == Colour::White) {
                white_bishops++;
            } else {
                black_bishops++;
            }
        }
        else if (piece.type == PieceType::Knight) {
            if (piece.colour == Colour::White) {
                white_knights++;
            } else {
                black_knights++;
            }
        }
        else {
            // Pawn, rook or queen is sufficient material
            return false;
        }
    }

    // Both sides have lone king, or king and bishop, or king and knight
    bool white_insufficient = (white_knights == 0 && white_bishops == 1)
                                || (white_knights == 1 && white_bishops == 0)
                                || (white_knights == 0 && white_bishops == 0);
    bool black_insufficient = (black_knights == 0 && black_bishops == 1)
                                || (black_knights == 1 && black_bishops == 0)
                                || (black_knights == 0 && black_bishops == 0);
    if (white_insufficient && black_insufficient) {
        return true;
    }

    // Two knights vs lone king
    if (white_knights == 2 && white_bishops == 0 && black_knights == 0 && black_bishops == 0) {
        return true;
    }
    if (black_knights == 2 && black_bishops == 0 && white_knights == 0 && white_bishops == 0) {
        return true;
    }

    return false;
}

int Board::countPieces() const {
    int count = 0;
    for (const Piece& piece : squares) {
        if (piece.type != PieceType::None) {
            count++;
        }
    }

    return count;
}

const Piece& Board::getPiece(int square) const { return squares[square]; }

const Piece& Board::getPiece(int rank, int file) const { return squares[rank * 8 + file]; }

void Board::setPiece(int square, Piece piece) {
    const Piece& captured_piece = squares[square];
    if (captured_piece.type != PieceType::None) {
        zobrist_key ^= Zobrist::piece[static_cast<int>(captured_piece.colour)][static_cast<int>(captured_piece.type)][square];
    }
    if (piece.type != PieceType::None) {
        zobrist_key ^= Zobrist::piece[static_cast<int>(piece.colour)][static_cast<int>(piece.type)][square];
    }

    squares[square] = piece;

    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            white_king_square = square;
        } else {
            black_king_square = square;
        }
    }
}

void Board::setPiece(int rank, int file, Piece piece) { setPiece(rank * 8 + file, piece); }

Colour Board::getTurn() const { return turn; }

void Board::setTurn(Colour colour) { turn = colour; }

CastleRights Board::getCastleRights(Colour colour) const {
    if (colour == Colour::White) {
        return white_castle_rights;
    } else {
        return black_castle_rights;
    }
}

void Board::setCastleRights(Colour colour, CastleRights rights) {
    if (colour == Colour::White) {
        white_castle_rights = rights;
    } else if (colour == Colour::Black) {
        black_castle_rights = rights;
    }
}

int Board::getEnPassantSquare() const { return en_passant_square; }

void Board::setEnPassantSquare(int square) { en_passant_square = square; }

const std::array<Piece, 64>& Board::getSquares() const { return squares; }

std::uint64_t Board::calculateZobristKey() {
    uint64_t key = 0;

    for (int i = 0; i < 64; i++) {
        const Piece& piece = squares[i];
        if (piece.type != PieceType::None) {
            key ^= Zobrist::piece[static_cast<int>(piece.colour)][static_cast<int>(piece.type)][i];
        }
    }

    if (turn == Colour::Black) {
        key ^= Zobrist::side_to_move;
    }

    key ^= Zobrist::castle_rights[0] * white_castle_rights.queen_side;
    key ^= Zobrist::castle_rights[1] * white_castle_rights.king_side;
    key ^= Zobrist::castle_rights[2] * black_castle_rights.queen_side;
    key ^= Zobrist::castle_rights[3] * black_castle_rights.king_side;

    if (en_passant_square != -1) {
        int file = en_passant_square % 8;
        key ^= Zobrist::en_passant[file];
    }

    return key;
}

std::uint64_t Board::getZobristKey() const { return zobrist_key; }

void Board::clear() {
    squares.fill(Piece());

    turn = Colour::White;

    white_castle_rights = {false, false};
    black_castle_rights = {false, false};

    white_king_square = -1;
    black_king_square = -1;

    en_passant_square = -1;
    
    move_history.clear();

    zobrist_key = 0;
}