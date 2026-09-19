#include "Game.h"
#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"
#include "CastleRights.h"
#include <sstream>
#include <stdexcept>
#include <cctype>

Game::Game() {
    current_legal_moves = move_generator.generateLegalMoves(board);
    trackPosition();
}

void Game::trackPosition() {
    current_position = getPositionKey();
    positions[current_position]++;
}

GameState Game::getGameState() const {
    Colour turn = board.getTurn();

    auto position = positions.find(getPositionKey());
    if (position != positions.end() && position->second >= 3) {
        return GameState::Draw;
    }

    bool in_check = board.isKingInCheck(turn);
    if (current_legal_moves.empty()) {
        if (in_check) {
            return GameState::Checkmate;
        }
        return GameState::Draw;
    }

    if (board.isInsufficientMaterial()) {
        return GameState::Draw;
    }

    if (halfmove_clock >= 100) {
        return GameState::Draw;
    }

    if (in_check) {
        return GameState::Check;
    }

    return GameState::Playing;
}

std::vector<Move> Game::getLegalMoves() const {
    // current_legal_moves = move_generator.generateLegalMoves(board);
    return current_legal_moves;
}

void Game::makeMove(const Move& move) {
    history.push_back({current_position, halfmove_clock, fullmove_number});

    const Piece& moving_piece = board.getPiece(move.from);
    const Piece& captured_piece = board.getPiece(move.to);
    Colour moving_colour = moving_piece.colour;

    if (moving_piece.type == PieceType::Pawn || captured_piece.type != PieceType::None || move.type == MoveType::EnPassant) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }

    board.makeMove(move);

    if (moving_colour == Colour::Black) {
        fullmove_number++;
    }

    current_legal_moves = move_generator.generateLegalMoves(board);
    trackPosition();
}

void Game::undoMove() {
    auto it = positions.find(current_position);
    if (it != positions.end()) {
        it->second--;

        if (it->second == 0) {
            positions.erase(it);
        }
    }

    board.undoMove();

    halfmove_clock = history.back().halfmove_clock;
    fullmove_number = history.back().fullmove_number;
    current_position = history.back().position_key;

    history.pop_back();

    current_legal_moves = move_generator.generateLegalMoves(board);
}

Colour Game::getTurn() const { return board.getTurn(); }

Board& Game::getBoard() { return board; }

const Board& Game::getBoard() const { return board; }

void Game::setPosition(const std::string& fen) {
    std::istringstream stream(fen);

    std::string board_part;
    std::string turn_part;
    std::string castling_part;
    std::string en_passant_part;
    int new_halfmove_clock;
    int new_fullmove_number;

    if (!(stream >> board_part
                 >> turn_part
                 >> castling_part
                 >> en_passant_part
                 >> new_halfmove_clock
                 >> new_fullmove_number)) {
        throw std::invalid_argument("Invalid FEN");
    }

    board.clear();
    halfmove_clock = 0;
    fullmove_number = 1;

    // Set board
    int rank = 7;
    int file = 0;

    for (char c : board_part) {
        if (c == '/') {
            if (file != 8 || rank == 0) {
                throw std::invalid_argument("Invalid FEN board");
            }

            rank--;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            file += c - '0';
            continue;
        }

        PieceType piece_type;
        Colour colour;

        if (c >= 'A' && c <= 'Z') {
            colour = Colour::White;
        } else if (c >= 'a' && c <= 'z') {
            colour = Colour::Black;
        } else {
            throw std::invalid_argument("Invalid FEN piece");
        }

        switch (std::tolower(c)) {
            case 'p':
                piece_type = PieceType::Pawn;
                break;
            case 'n':
                piece_type = PieceType::Knight;
                break;
            case 'b':
                piece_type = PieceType::Bishop;
                break;
            case 'r':
                piece_type = PieceType::Rook;
                break;
            case 'q':
                piece_type = PieceType::Queen;
                break;
            case 'k':
                piece_type = PieceType::King;
                break;
            default:
                throw std::invalid_argument("Invalid FEN piece");
        }

        if (file >= 8) {
            throw std::invalid_argument("Invalid FEN board");
        }

        board.setPiece(rank, file, Piece(piece_type, colour));

        file++;
    }

    if (rank != 0 || file != 8) {
        throw std::invalid_argument("Invalid FEN board");
    }


    // Side to move
    if (turn_part == "w") {
        board.setTurn(Colour::White);
    } else if (turn_part == "b") {
        board.setTurn(Colour::Black);
    } else {
        throw std::invalid_argument("Invalid FEN turn");
    }


    // Castling rights
    board.setCastleRights(Colour::White, {false, false});
    board.setCastleRights(Colour::Black, {false, false});
    if (castling_part != "-") {
        for (char c : castling_part) {
            switch (c) {
                case 'K':
                    board.setCastleRights(
                        Colour::White,
                        {true, board.getCastleRights(Colour::White).queen_side}
                    );
                    break;

                case 'Q':
                    board.setCastleRights(
                        Colour::White,
                        {board.getCastleRights(Colour::White).king_side, true}
                    );
                    break;

                case 'k':
                    board.setCastleRights(
                        Colour::Black,
                        {true, board.getCastleRights(Colour::Black).queen_side}
                    );
                    break;

                case 'q':
                    board.setCastleRights(
                        Colour::Black,
                        {board.getCastleRights(Colour::Black).king_side, true}
                    );
                    break;

                default:
                    throw std::invalid_argument("Invalid FEN castling rights");
            }
        }
    }


    // En passant
    if (en_passant_part == "-") {
        board.setEnPassantSquare(-1);
    } else {
        if (en_passant_part.size() != 2) {
            throw std::invalid_argument("Invalid FEN en passant");
        }

        int en_passant_file = en_passant_part[0] - 'a';
        int en_passant_rank = en_passant_part[1] - '1';

        if (en_passant_file < 0 || en_passant_file >= 8 || en_passant_rank < 0 || en_passant_rank >= 8) {
            throw std::invalid_argument("Invalid FEN en passant");
        }

        board.setEnPassantSquare(
            en_passant_rank * 8 + en_passant_file
        );
    }


    if (new_halfmove_clock < 0 || new_fullmove_number < 1) {
        throw std::invalid_argument("Invalid FEN move counters");
    }
    halfmove_clock = new_halfmove_clock;
    fullmove_number = new_fullmove_number;

    positions.clear();
    history.clear();

    current_legal_moves = move_generator.generateLegalMoves(board);

    current_position = getPositionKey();

    positions[current_position] = 1;
}

std::string Game::getPositionKey() const {
    std::string key;

    // Board position
    for (int rank = 7; rank >= 0; rank--) {
        int empty_squares = 0;

        for (int file = 0; file < 8; file++) {
            const Piece& piece = board.getPiece(rank, file);

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
    if (board.getTurn() == Colour::White) {
        key += " w ";
    } else {
        key += " b ";
    }

    // Castling rights
    bool has_castling_rights = false;
    CastleRights white_castle_rights = board.getCastleRights(Colour::White);
    CastleRights black_castle_rights = board.getCastleRights(Colour::Black);
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
    for (const Move& move : current_legal_moves) {
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

std::string Game::getPositionFen() const {
    std::string fen = getPositionKey();
    if (fen.back() == '-') {
        int en_passant_square = board.getEnPassantSquare();
        if (en_passant_square != -1) {
            fen.pop_back();

            int target_rank = en_passant_square / 8;
            int target_file = en_passant_square % 8;

            fen += static_cast<char>('a' + target_file);
            fen += static_cast<char>('1' + target_rank);
        }
    }

    fen += " " + std::to_string(halfmove_clock);
    fen += " " + std::to_string(fullmove_number);

    return fen;
}