#include "Game.h"
#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"
#include <sstream>
#include <stdexcept>
#include <cctype>

Game::Game() {
    current_legal_moves = move_generator.generateLegalMoves(board);
    current_position = board.getPositionKey(current_legal_moves);
}

void Game::trackPosition(const std::vector<Move>& legal_moves) {
    current_position = board.getPositionKey(legal_moves);
    positions[current_position]++;
}

GameState Game::getGameState(const std::vector<Move>& legal_moves) const {
    Colour turn = board.getTurn();

    auto position = positions.find(board.getPositionKey(legal_moves));
    if (position != positions.end() && position->second >= 3) {
        return GameState::Draw;
    }

    bool in_check = board.isKingInCheck(turn);
    if (legal_moves.empty()) {
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

std::vector<Move> Game::getLegalMoves() {
    // current_legal_moves = move_generator.generateLegalMoves(board);
    return current_legal_moves;
}

void Game::makeMove(const Move& move) {
    history.push_back({current_position, halfmove_clock});

    const Piece& moving_piece = board.getPiece(move.from);
    const Piece& captured_piece = board.getPiece(move.to);
    if (moving_piece.type == PieceType::Pawn || captured_piece.type != PieceType::None || move.type == MoveType::EnPassant) {
        halfmove_clock = 0;
    } else {
        halfmove_clock++;
    }

    board.makeMove(move);

    current_legal_moves = move_generator.generateLegalMoves(board);
    trackPosition(current_legal_moves);
}

void Game::undoMove() {
    board.undoMove();

    const std::string& position = history.back().position_key;
    positions[position]--;
    if (positions[position] == 0) {
        positions.erase(position);
    }

    halfmove_clock = history.back().halfmove_clock;

    history.pop_back();
}

Colour Game::getTurn() const { return board.getTurn(); }

const Board& Game::getBoard() const { return board; }

void Game::setPosition(const std::string& fen) {
    std::istringstream stream(fen);

    std::string board_part;
    std::string turn_part;
    std::string castling_part;
    std::string en_passant_part;
    int new_halfmove_clock;
    int fullmove_number;

    if (!(stream >> board_part
                 >> turn_part
                 >> castling_part
                 >> en_passant_part
                 >> new_halfmove_clock
                 >> fullmove_number)) {
        throw std::invalid_argument("Invalid FEN");
    }

    board.clear();

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

        board.setPiece(
            rank * 8 + file,
            Piece(piece_type, colour)
        );

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


    halfmove_clock = new_halfmove_clock;

    positions.clear();
    history.clear();

    current_legal_moves = move_generator.generateLegalMoves(board);

    current_position = board.getPositionKey(current_legal_moves);

    positions[current_position] = 1;
}
