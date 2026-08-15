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

const Piece& Board::getPiece(int square) const { return squares[square]; }

const Piece& Board::getPiece(int rank, int file) const { return squares[rank * 8 + file]; }

void Board::setPiece(int rank, int file, Piece piece) { squares[rank * 8 + file] = piece; }

Colour Board::getTurn() const { return turn; }

void Board::setTurn(Colour colour) { turn = colour; }

void Board::clear() {
    for (auto square : squares) {
        square = Piece();
    }
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