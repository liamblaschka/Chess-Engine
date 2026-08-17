#include "Board.h"
#include "Piece.h"
#include "SquareConstants.h"
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

    turn = Colour::White;
    white_castle_rights = {true, true};
    black_castle_rights = {true, true};
}

void Board::makeMove(const Move& move) {
    // Move
    switch (move.type) {
        case MoveType::Normal: {
            move_history.push_back({move, squares[move.to], move.to, white_castle_rights, black_castle_rights});

            // Castle rights if rook is captured
            const Piece& captured_piece = squares[move.to];
            if (captured_piece.type == PieceType::Rook) {
                if (captured_piece.colour == Colour::White) {
                    if (move.to == Square::A1) {
                        white_castle_rights.queen_side = false;
                    } else if (move.to == Square::H1) {
                        white_castle_rights.king_side = false;
                    }
                } else if (captured_piece.colour == Colour::Black) {
                    if (move.to == Square::A8) {
                        black_castle_rights.queen_side = false;
                    } else if (move.to == Square::H8) {
                        black_castle_rights.king_side = false;
                    }
                }
            }

            squares[move.to] = squares[move.from];
            squares[move.from] = Piece();
            break;
        }
        case MoveType::Castle: {
            Colour colour = squares[move.from].colour;

            move_history.push_back({move, squares[move.to], move.to, white_castle_rights, black_castle_rights});

            squares[move.to] = squares[move.from];
            squares[move.from] = Piece();

            if (colour == Colour::White) {
                if (move.to == Square::C1) {
                    // Queen-side
                    squares[Square::D1] = squares[Square::A1];
                    squares[Square::A1] = Piece();
                } else {
                    // King-side
                    squares[Square::F1] = squares[Square::H1];
                    squares[Square::H1] = Piece();
                }
            } else {
                if (move.to == Square::C8) {
                    // Queen-side
                    squares[Square::D8] = squares[Square::A8];
                    squares[Square::A8] = Piece();
                } else {
                    // King-side
                    squares[Square::F8] = squares[Square::H8];
                    squares[Square::H8] = Piece();
                }
            }
            break;
        }
        case MoveType::EnPassant: {
            int from_rank = move.from / 8;
            int to_rank = move.to / 8;
            int direction = to_rank - from_rank;
            int captured_square = move.to - (8 * direction);
            move_history.push_back({move, squares[captured_square], captured_square, white_castle_rights, black_castle_rights});

            squares[move.to] = squares[move.from];
            squares[move.from] = Piece();
            squares[captured_square] = Piece();
            break;
        }
        case MoveType::Promotion: {
            move_history.push_back({move, squares[move.to], move.to, white_castle_rights, black_castle_rights});

            squares[move.to] = move.promotion_piece;
            squares[move.from] = Piece();
            break;
        }
    }

    // Update castling rights
    const Piece& piece = squares[move.to];
    if (piece.type == PieceType::King) {
        if (piece.colour == Colour::White) {
            white_castle_rights.queen_side = false;
            white_castle_rights.king_side = false;
        } else {
            black_castle_rights.queen_side = false;
            black_castle_rights.king_side = false;
        }
    } else if (piece.type == PieceType::Rook) {
        if (piece.colour == Colour::White && (move.from == Square::A1 || move.from == Square::H1)) {
            if (move.from == Square::A1) {
                white_castle_rights.queen_side = false;
            } else if (move.from == Square::H1) {
                white_castle_rights.king_side = false;
            }
        } else if (move.from == Square::A8 || move.from == Square::H8) {
            if (move.from == Square::A8) {
                black_castle_rights.queen_side = false;
            } else if (move.from == Square::H8) {
                black_castle_rights.king_side = false;
            }
        }
    }

    turn = oppositeColour(turn);
}

void Board::undoMove() {
    MoveState previous = move_history.back();
    move_history.pop_back();

    switch (previous.move.type) {
        case MoveType::Normal:
            squares[previous.move.from] = squares[previous.move.to];
            squares[previous.move.to] = previous.captured_piece;
            break;
        case MoveType::Castle:
            if (squares[previous.move.to].colour == Colour::White) {
                if (previous.move.to == Square::C1) {
                    // Queen-side
                    squares[Square::A1] = squares[Square::D1];
                    squares[Square::D1] = Piece();
                } else {
                    // King-side
                    squares[Square::H1] = squares[Square::F1];
                    squares[Square::F1] = Piece();
                }
            } else {
                if (previous.move.to == Square::C8) {
                    // Queen-side
                    squares[Square::A8] = squares[Square::D8];
                    squares[Square::D8] = Piece();
                } else {
                    // King-side
                    squares[Square::H8] = squares[Square::F8];
                    squares[Square::F8] = Piece();
                }
            }

            squares[previous.move.from] = squares[previous.move.to];
            squares[previous.move.to] = Piece();

            break;
        case MoveType::EnPassant:
            squares[previous.move.from] = squares[previous.move.to];
            squares[previous.move.to] = Piece();
            squares[previous.captured_square] = previous.captured_piece;
            break;
        case MoveType::Promotion:
            squares[previous.move.from] = Piece(PieceType::Pawn, previous.move.promotion_piece.colour);
            squares[previous.move.to] = previous.captured_piece;
            break;
    }
    
    white_castle_rights = previous.white_castle_rights;
    black_castle_rights = previous.black_castle_rights;

    turn = oppositeColour(turn);
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

    return isSquareAttacked(king_rank, king_file, oppositeColour(colour));   
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

std::string Board::getPositionKey(const std::vector<Move>& legal_moves) const {
    std::string key;

    // Board position
    for (int rank = 7; rank >= 0; rank--) {
        int empty_squares = 0;

        for (int file = 0; file < 8; file++) {
            const Piece& piece = squares[rank * 8 + file];

            if (piece.type == PieceType::None) {
                empty_squares++;
                continue;
            }

            if (empty_squares > 0) {
                key += std::to_string(empty_squares);
                empty_squares = 0;
            }

            key += piece.getSymbol();
        }

        if (empty_squares > 0) {
            key += std::to_string(empty_squares);
        }

        if (rank > 0) {
            key += '/';
        }
    }

    // Turn
    if (turn == Colour::White) {
        key += " w ";
    } else {
        key += " b ";
    }

    // Castling rights
    bool has_castling_rights = false;
    if (white_castle_rights.king_side) {
        key += 'K';
        has_castling_rights = true;
    }
    if (white_castle_rights.queen_side) {
        key += 'Q';
        has_castling_rights = true;
    }
    if (black_castle_rights.king_side) {
        key += 'k';
        has_castling_rights = true;
    }
    if (black_castle_rights.queen_side) {
        key += 'q';
        has_castling_rights = true;
    }
    if (!has_castling_rights) {
        key += '-';
    }

    // En passant
    key += ' ';
    bool en_passant_available = false;
    for (const Move& move : legal_moves) {
        if (move.type == MoveType::EnPassant) {
            int target_rank = move.to / 8;
            int target_file = move.to % 8;

            key += static_cast<char>('a' + target_file);
            key += static_cast<char>('1' + target_rank);

            en_passant_available = true;
            break;
        }
    }
    if (!en_passant_available) {
        key += '-';
    }

    return key;
}

const Piece& Board::getPiece(int square) const { return squares[square]; }

const Piece& Board::getPiece(int rank, int file) const { return squares[rank * 8 + file]; }

void Board::setPiece(int square, Piece piece) { squares[square] = piece; }

void Board::setPiece(int rank, int file, Piece piece) { squares[rank * 8 + file] = piece; }

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

void Board::clear() {
    squares.fill(Piece());
}

const std::array<Piece, 64>& Board::getSquares() const { return squares; }

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