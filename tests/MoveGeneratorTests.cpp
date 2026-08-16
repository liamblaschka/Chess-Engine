#include "Board.h"
#include "MoveGenerator.h"
#include "SquareConstants.h"
#include "Piece.h"
#include <iostream>
#include <string>
#include <vector>

bool containsMove(
    const std::vector<Move>& moves,
    int from_rank,
    int from_file,
    int to_rank,
    int to_file
) {
    int from = from_rank * 8 + from_file;
    int to = to_rank * 8 + to_file;

    for (const Move& move : moves) {
        if (move.from == from && move.to == to) {
            return true;
        }
    }

    return false;
}

void testMove(
    const std::string& name,
    const std::vector<Move>& moves,
    int from_rank,
    int from_file,
    int to_rank,
    int to_file
) {
    if (containsMove(moves, from_rank, from_file, to_rank, to_file)) {
        std::cout << "[PASS] " << name << '\n';
    } else {
        std::cout << "[FAIL] " << name << '\n';
    }
}

void testMoveAbsent(
    const std::string& name,
    const std::vector<Move>& moves,
    int from_rank,
    int from_file,
    int to_rank,
    int to_file
) {
    if (!containsMove(moves, from_rank, from_file, to_rank, to_file)) {
        std::cout << "[PASS] " << name << '\n';
    } else {
        std::cout << "[FAIL] " << name << '\n';
    }
}

void testPawnMoves(MoveGenerator& generator) {
    std::cout << "\n--- Pawn Tests ---\n";

    // White pawn: one and two squares
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8
        board.setPiece(1, 4, {PieceType::Pawn, Colour::White}); // e2

        auto moves = generator.generateLegalMoves(board);

        testMove("White pawn one square", moves, 1, 4, 2, 4);
        testMove("White pawn two squares", moves, 1, 4, 3, 4);
    }

    // Black pawn: one and two squares
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8
        board.setPiece(6, 4, {PieceType::Pawn, Colour::Black}); // e7

        auto moves = generator.generateLegalMoves(board);

        testMove("Black pawn one square", moves, 6, 4, 5, 4);
        testMove("Black pawn two squares", moves, 6, 4, 4, 4);
    }

    // White pawn captures
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White}); // e4
        board.setPiece(4, 3, {PieceType::Pawn, Colour::Black}); // d5
        board.setPiece(4, 5, {PieceType::Pawn, Colour::Black}); // f5

        auto moves = generator.generateLegalMoves(board);

        testMove("White pawn captures left", moves, 3, 4, 4, 3);
        testMove("White pawn captures right", moves, 3, 4, 4, 5);
    }

    // Black pawn captures
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(4, 4, {PieceType::Pawn, Colour::Black}); // e5
        board.setPiece(3, 3, {PieceType::Pawn, Colour::White}); // d4
        board.setPiece(3, 5, {PieceType::Pawn, Colour::White}); // f4

        auto moves = generator.generateLegalMoves(board);

        testMove("Black pawn captures left", moves, 4, 4, 3, 3);
        testMove("Black pawn captures right", moves, 4, 4, 3, 5);
    }

    // Pawn cannot capture empty square
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8
        board.setPiece(3, 4, {PieceType::Pawn, Colour::White}); // e4

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent("Pawn cannot capture empty left", moves, 3, 4, 4, 3);
        testMoveAbsent("Pawn cannot capture empty right", moves, 3, 4, 4, 5);
    }

    // Pawn blocked
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(1, 4, {PieceType::Pawn, Colour::White});
        board.setPiece(2, 4, {PieceType::Pawn, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent("Pawn cannot move when blocked", moves, 1, 4, 2, 4);
        testMoveAbsent("Pawn cannot double move when blocked", moves, 1, 4, 3, 4);
    }

    // Pawn cannot capture friendly piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White});
        board.setPiece(4, 3, {PieceType::Pawn, Colour::White});
        board.setPiece(4, 5, {PieceType::Pawn, Colour::White});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Pawn cannot capture friendly left",
            moves, 3, 4, 4, 3
        );

        testMoveAbsent(
            "Pawn cannot capture friendly right",
            moves, 3, 4, 4, 5
        );
    }
}

void testEnPassantMoves(MoveGenerator& generator) {
    std::cout << "\n--- En Passant Tests ---\n";

    // White captures black pawn en passant to the left
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black}); // d7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d5
        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "White can capture en passant left",
            moves,
            4, 4,
            5, 3
        );
    }

    // White captures black pawn en passant to the right
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(6, 5, {PieceType::Pawn, Colour::Black}); // f7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: f7 -> f5
        board.makeMove(
            Move(6, 5, 4, 5, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "White can capture en passant right",
            moves,
            4, 4,
            5, 5
        );
    }

    // Black captures white pawn en passant to the left
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(1, 3, {PieceType::Pawn, Colour::White}); // d2
        board.setPiece(3, 4, {PieceType::Pawn, Colour::Black}); // e4

        // White: d2 -> d4
        board.makeMove(
            Move(1, 3, 3, 3, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Black can capture en passant left",
            moves,
            3, 4,
            2, 3
        );
    }

    // Black captures white pawn en passant to the right
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(1, 5, {PieceType::Pawn, Colour::White}); // f2
        board.setPiece(3, 4, {PieceType::Pawn, Colour::Black}); // e4

        // White: f2 -> f4
        board.makeMove(
            Move(1, 5, 3, 5, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Black can capture en passant right",
            moves,
            3, 4,
            2, 5
        );
    }

    // Cannot en passant after a one-square pawn move
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(5, 3, {PieceType::Pawn, Colour::Black}); // d6
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d6
        board.makeMove(
            Move(6, 3, 5, 3, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "White cannot en passant after one-square move",
            moves,
            4, 4,
            5, 3
        );
    }

    // Cannot en passant after another move has been made
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White}); // a1
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black}); // d7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        board.setPiece(1, 0, {PieceType::Pawn, Colour::White}); // a2
        board.setPiece(6, 0, {PieceType::Pawn, Colour::Black}); // a7

        // Black: d7 -> d5
        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        // White: a2 -> a3
        board.makeMove(
            Move(1, 0, 2, 0, MoveType::Normal)
        );

        // Black: a7 -> a6
        board.makeMove(
            Move(6, 0, 5, 0, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "White cannot en passant after another move",
            moves,
            4, 4,
            5, 3
        );
    }

    // En passant move has the correct MoveType
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White});
        board.setPiece(7, 7, {PieceType::King, Colour::Black});

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black});
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        auto moves = generator.generateLegalMoves(board);

        bool found_en_passant = false;

        for (const Move& move : moves) {
            if (move.from == 4 * 8 + 4 &&
                move.to == 5 * 8 + 3 &&
                move.type == MoveType::EnPassant) {
                found_en_passant = true;
                break;
            }
        }

        if (found_en_passant) {
            std::cout << "[PASS] En passant move has correct type\n";
        } else {
            std::cout << "[FAIL] En passant move has correct type\n";
        }
    }

    // En passant actually captures the pawn
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White});
        board.setPiece(7, 7, {PieceType::King, Colour::Black});

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black});
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        board.makeMove(
            Move(4, 4, 5, 3, MoveType::EnPassant)
        );

        const Piece& destination = board.getPiece(5, 3);
        const Piece& captured = board.getPiece(4, 3);

        if (destination.type == PieceType::Pawn &&
            destination.colour == Colour::White &&
            captured.type == PieceType::None) {
            std::cout << "[PASS] En passant captures pawn\n";
        } else {
            std::cout << "[FAIL] En passant captures pawn\n";
        }
    }

    // Undo en passant restores the captured pawn
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(0, 0, {PieceType::King, Colour::White});
        board.setPiece(7, 7, {PieceType::King, Colour::Black});

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black});
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        board.makeMove(
            Move(4, 4, 5, 3, MoveType::EnPassant)
        );

        board.undoMove();

        const Piece& white_pawn = board.getPiece(4, 4);
        const Piece& black_pawn = board.getPiece(4, 3);

        if (white_pawn.type == PieceType::Pawn &&
            white_pawn.colour == Colour::White &&
            black_pawn.type == PieceType::Pawn &&
            black_pawn.colour == Colour::Black) {
            std::cout << "[PASS] Undo en passant restores board\n";
        } else {
            std::cout << "[FAIL] Undo en passant restores board\n";
        }
    }
}

void testKnightMoves(MoveGenerator& generator) {
    std::cout << "\n--- Knight Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(0, 0, {PieceType::King, Colour::White});
    board.setPiece(7, 7, {PieceType::King, Colour::Black});
    board.setPiece(3, 3, {PieceType::Knight, Colour::White}); // d4

    auto moves = generator.generateLegalMoves(board);

    testMove("Knight up-right", moves, 3, 3, 5, 4);
    testMove("Knight up-left", moves, 3, 3, 5, 2);
    testMove("Knight down-right", moves, 3, 3, 1, 4);
    testMove("Knight down-left", moves, 3, 3, 1, 2);
    testMove("Knight right-up", moves, 3, 3, 4, 5);
    testMove("Knight right-down", moves, 3, 3, 2, 5);
    testMove("Knight left-up", moves, 3, 3, 4, 1);
    testMove("Knight left-down", moves, 3, 3, 2, 1);

    board.setPiece(5, 4, {PieceType::Pawn, Colour::White});

    moves = generator.generateLegalMoves(board);

    testMoveAbsent(
        "Knight cannot capture friendly piece",
        moves, 3, 3, 5, 4
    );

    board.setPiece(5, 2, {PieceType::Pawn, Colour::Black});

    moves = generator.generateLegalMoves(board);

    testMove(
        "Knight can capture enemy piece",
        moves, 3, 3, 5, 2
    );
}

void testBishopMoves(MoveGenerator& generator) {
    std::cout << "\n--- Bishop Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(0, 0, {PieceType::King, Colour::White});
    board.setPiece(7, 7, {PieceType::King, Colour::Black});
    board.setPiece(3, 3, {PieceType::Bishop, Colour::White}); // d4

    auto moves = generator.generateLegalMoves(board);

    testMove("Bishop up-right", moves, 3, 3, 4, 4);
    testMove("Bishop up-left", moves, 3, 3, 4, 2);
    testMove("Bishop down-right", moves, 3, 3, 2, 4);
    testMove("Bishop down-left", moves, 3, 3, 2, 2);

    testMove("Bishop two squares up-right", moves, 3, 3, 5, 5);
    testMove("Bishop two squares down-left", moves, 3, 3, 1, 1);

    board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

    moves = generator.generateLegalMoves(board);

    testMoveAbsent(
        "Bishop cannot move through friendly piece",
        moves, 3, 3, 5, 5
    );

    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(0, 0, {PieceType::King, Colour::White});
    board.setPiece(7, 7, {PieceType::King, Colour::Black});
    board.setPiece(3, 3, {PieceType::Bishop, Colour::White});
    board.setPiece(5, 5, {PieceType::Pawn, Colour::Black});

    moves = generator.generateLegalMoves(board);

    testMove("Bishop can capture enemy piece", moves, 3, 3, 5, 5);
    testMoveAbsent("Bishop cannot move through enemy piece", moves, 3, 3, 6, 6);
}

void testRookMoves(MoveGenerator& generator) {
    std::cout << "\n--- Rook Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(0, 0, {PieceType::King, Colour::White});
    board.setPiece(7, 7, {PieceType::King, Colour::Black});
    board.setPiece(3, 3, {PieceType::Rook, Colour::White}); // d4

    auto moves = generator.generateLegalMoves(board);

    testMove("Rook up", moves, 3, 3, 4, 3);
    testMove("Rook down", moves, 3, 3, 2, 3);
    testMove("Rook left", moves, 3, 3, 3, 2);
    testMove("Rook right", moves, 3, 3, 3, 4);

    testMove("Rook two squares up", moves, 3, 3, 5, 3);
    testMove("Rook two squares right", moves, 3, 3, 3, 5);

    board.setPiece(4, 3, {PieceType::Pawn, Colour::White});

    moves = generator.generateLegalMoves(board);

    testMoveAbsent(
        "Rook cannot move through friendly piece",
        moves, 3, 3, 5, 3
    );

    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(0, 0, {PieceType::King, Colour::White});
    board.setPiece(7, 7, {PieceType::King, Colour::Black});
    board.setPiece(3, 3, {PieceType::Rook, Colour::White});
    board.setPiece(3, 5, {PieceType::Pawn, Colour::Black});

    moves = generator.generateLegalMoves(board);

    testMove("Rook can capture enemy piece", moves, 3, 3, 3, 5);
    testMoveAbsent("Rook cannot move through enemy piece", moves, 3, 3, 3, 6);
}

void testQueenMoves(MoveGenerator& generator) {
    std::cout << "\n--- Queen Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(0, 0, {PieceType::King, Colour::White});
    board.setPiece(7, 7, {PieceType::King, Colour::Black});
    board.setPiece(3, 3, {PieceType::Queen, Colour::White}); // d4

    auto moves = generator.generateLegalMoves(board);

    testMove("Queen up", moves, 3, 3, 4, 3);
    testMove("Queen down", moves, 3, 3, 2, 3);
    testMove("Queen left", moves, 3, 3, 3, 2);
    testMove("Queen right", moves, 3, 3, 3, 4);

    testMove("Queen up-right", moves, 3, 3, 4, 4);
    testMove("Queen up-left", moves, 3, 3, 4, 2);
    testMove("Queen down-right", moves, 3, 3, 2, 4);
    testMove("Queen down-left", moves, 3, 3, 2, 2);

    testMove("Queen two squares up", moves, 3, 3, 5, 3);
    testMove("Queen two squares up-right", moves, 3, 3, 5, 5);

    board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

    moves = generator.generateLegalMoves(board);

    testMoveAbsent(
        "Queen cannot move through friendly piece",
        moves, 3, 3, 5, 5
    );
}

void testKingMoves(MoveGenerator& generator) {
    std::cout << "\n--- King Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);

    board.setPiece(3, 3, {PieceType::King, Colour::White}); // d4
    board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

    auto moves = generator.generateLegalMoves(board);

    testMove("King up", moves, 3, 3, 4, 3);
    testMove("King down", moves, 3, 3, 2, 3);
    testMove("King left", moves, 3, 3, 3, 2);
    testMove("King right", moves, 3, 3, 3, 4);

    testMove("King up-right", moves, 3, 3, 4, 4);
    testMove("King up-left", moves, 3, 3, 4, 2);
    testMove("King down-right", moves, 3, 3, 2, 4);
    testMove("King down-left", moves, 3, 3, 2, 2);

    board.setPiece(4, 3, {PieceType::Pawn, Colour::White});

    moves = generator.generateLegalMoves(board);

    testMoveAbsent(
        "King cannot capture friendly piece",
        moves, 3, 3, 4, 3
    );

    board.setPiece(2, 3, {PieceType::Pawn, Colour::Black});

    moves = generator.generateLegalMoves(board);

    testMove(
        "King can capture enemy piece",
        moves, 3, 3, 2, 3
    );
}

void testCastlingMoves(MoveGenerator& generator) {
    std::cout << "\n--- Castling Tests ---\n";

    // White can castle king-side
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "White can castle king-side",
            moves,
            0, 4,
            0, 6
        );
    }

    // White can castle queen-side
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "White can castle queen-side",
            moves,
            0, 4,
            0, 2
        );
    }

    // Black can castle king-side
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(Square::E8, {PieceType::King, Colour::Black});
        board.setPiece(Square::H8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::E1, {PieceType::King, Colour::White});

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Black can castle king-side",
            moves,
            7, 4,
            7, 6
        );
    }

    // Black can castle queen-side
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(Square::E8, {PieceType::King, Colour::Black});
        board.setPiece(Square::A8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::E1, {PieceType::King, Colour::White});

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Black can castle queen-side",
            moves,
            7, 4,
            7, 2
        );
    }

    // King-side castling blocked by piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::F1, {PieceType::Knight, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Cannot castle king-side when F1 is occupied",
            moves,
            0, 4,
            0, 6
        );
    }

    // Queen-side castling blocked by piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::D1, {PieceType::Knight, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Cannot castle queen-side when D1 is occupied",
            moves,
            0, 4,
            0, 2
        );
    }

    // Cannot castle while in check
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::A8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Cannot castle while in check",
            moves,
            0, 4,
            0, 6
        );
    }

    // Cannot castle through an attacked square
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::F8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::A8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Cannot castle through attacked F1",
            moves,
            0, 4,
            0, 6
        );
    }

    // Cannot castle into an attacked square
    //
    // IMPORTANT:
    // This is deliberately tested through generateLegalMoves().
    // We do NOT independently test G1 for attack here because that
    // is the responsibility of the legal move generator.
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::G8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::A8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Cannot castle into attacked G1",
            moves,
            0, 4,
            0, 6
        );
    }

    // Cannot queen-side castle through attacked square
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::D8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::H8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Cannot castle through attacked D1",
            moves,
            0, 4,
            0, 2
        );
    }

    // B1 does not need to be safe for queen-side castling
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::B8, {PieceType::Rook, Colour::Black});
        board.setPiece(Square::H8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Queen-side castling allowed when B1 is attacked",
            moves,
            0, 4,
            0, 2
        );
    }

    // Correct MoveType
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        auto moves = generator.generateLegalMoves(board);

        bool found_castle = false;

        for (const Move& move : moves) {
            if (move.from == Square::E1 &&
                move.to == Square::G1 &&
                move.type == MoveType::Castle) {
                found_castle = true;
                break;
            }
        }

        if (found_castle) {
            std::cout << "[PASS] King-side castle has correct MoveType\n";
        } else {
            std::cout << "[FAIL] King-side castle has correct MoveType\n";
        }
    }

    // Castling actually moves king and rook
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});

        board.makeMove(
            Move(Square::E1, Square::G1, MoveType::Castle)
        );

        const Piece& king = board.getPiece(Square::G1);
        const Piece& rook = board.getPiece(Square::F1);

        if (king.type == PieceType::King &&
            king.colour == Colour::White &&
            rook.type == PieceType::Rook &&
            rook.colour == Colour::White &&
            board.getPiece(Square::E1).type == PieceType::None &&
            board.getPiece(Square::H1).type == PieceType::None) {
            std::cout << "[PASS] King-side castling moves king and rook\n";
        } else {
            std::cout << "[FAIL] King-side castling moves king and rook\n";
        }
    }

    // Undo castling
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});

        board.makeMove(
            Move(Square::E1, Square::G1, MoveType::Castle)
        );

        board.undoMove();

        const Piece& king = board.getPiece(Square::E1);
        const Piece& rook = board.getPiece(Square::H1);

        if (king.type == PieceType::King &&
            king.colour == Colour::White &&
            rook.type == PieceType::Rook &&
            rook.colour == Colour::White &&
            board.getPiece(Square::G1).type == PieceType::None &&
            board.getPiece(Square::F1).type == PieceType::None) {
            std::cout << "[PASS] Undo castling restores board\n";
        } else {
            std::cout << "[FAIL] Undo castling restores board\n";
        }
    }

    // Moving the king removes castling rights
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        board.makeMove(
            Move(Square::E1, Square::F1, MoveType::Normal)
        );

        board.setTurn(Colour::White);

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Moving king removes king-side castling",
            moves,
            0, 4,
            0, 6
        );
    }

    // Undoing a king move restores castling rights
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        board.makeMove(
            Move(Square::E1, Square::F1, MoveType::Normal)
        );

        board.undoMove();

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Undo king move restores king-side castling",
            moves,
            0, 4,
            0, 6
        );

        testMove(
            "Undo king move restores queen-side castling",
            moves,
            0, 4,
            0, 2
        );
    }

    // Moving king removes both castling rights
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        board.makeMove(
            Move(Square::E1, Square::F1, MoveType::Normal)
        );

        board.undoMove();

        board.makeMove(
            Move(Square::E1, Square::F1, MoveType::Normal)
        );

        board.setTurn(Colour::White);

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Moved king cannot castle king-side",
            moves,
            0, 4,
            0, 6
        );

        testMoveAbsent(
            "Moved king cannot castle queen-side",
            moves,
            0, 4,
            0, 2
        );
    }

    // Moving king does not permanently remove rights after undo
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        board.makeMove(
            Move(Square::E1, Square::F1, MoveType::Normal)
        );

        board.undoMove();

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Undo restores king-side castling rights",
            moves,
            0, 4,
            0, 6
        );

        testMove(
            "Undo restores queen-side castling rights",
            moves,
            0, 4,
            0, 2
        );
    }

    // Moving king-side rook removes king-side castling
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        board.makeMove(
            Move(Square::H1, Square::G1, MoveType::Normal)
        );

        board.setTurn(Colour::White);

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Moving H1 rook removes king-side castling",
            moves,
            0, 4,
            0, 6
        );

        testMove(
            "Moving H1 rook preserves queen-side castling",
            moves,
            0, 4,
            0, 2
        );
    }

    // Moving queen-side rook removes queen-side castling
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});

        board.makeMove(
            Move(Square::A1, Square::B1, MoveType::Normal)
        );

        board.setTurn(Colour::White);

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Moving A1 rook preserves king-side castling",
            moves,
            0, 4,
            0, 6
        );

        testMoveAbsent(
            "Moving A1 rook removes queen-side castling",
            moves,
            0, 4,
            0, 2
        );
    }

    // Capturing H1 rook removes king-side castling
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::H1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});
        board.setPiece(Square::G8, {PieceType::Rook, Colour::Black});

        board.makeMove(
            Move(Square::G8, Square::H1, MoveType::Normal)
        );

        board.setTurn(Colour::White);

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Capturing H1 rook removes king-side castling",
            moves,
            0, 4,
            0, 6
        );
    }

    // Capturing A1 rook removes queen-side castling
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(Square::E1, {PieceType::King, Colour::White});
        board.setPiece(Square::A1, {PieceType::Rook, Colour::White});
        board.setPiece(Square::E8, {PieceType::King, Colour::Black});
        board.setPiece(Square::B8, {PieceType::Rook, Colour::Black});

        board.makeMove(
            Move(Square::B8, Square::A1, MoveType::Normal)
        );

        board.setTurn(Colour::White);

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "Capturing A1 rook removes queen-side castling",
            moves,
            0, 4,
            0, 2
        );
    }
}

void testPseudoLegalMoves(MoveGenerator& generator) {
    std::cout << "\n--- Pseudo-Legal Move Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);

    // Multiple white pieces
    board.setPiece(3, 3, {PieceType::Rook, Colour::White});
    board.setPiece(1, 1, {PieceType::Knight, Colour::White});

    // Black pieces should not generate moves
    board.setPiece(5, 5, {PieceType::Bishop, Colour::Black});

    auto moves = generator.generatePseudoLegalMoves(board);

    testMove("Pseudo-legal rook move", moves, 3, 3, 4, 3);
    testMove("Pseudo-legal knight move", moves, 1, 1, 3, 2);

    testMoveAbsent(
        "Black piece does not generate moves",
        moves, 5, 5, 4, 4
    );
}

void testLegalMoves(MoveGenerator& generator) {
    std::cout << "\n--- Legal Move Tests ---\n";

    // A pinned rook should not be allowed to move off the file
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(0, 4, {PieceType::King, Colour::White}); // e1
        board.setPiece(1, 4, {PieceType::Rook, Colour::White}); // e2
        board.setPiece(7, 4, {PieceType::Rook, Colour::Black}); // e8

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "Pinned rook can move along pin",
            moves, 1, 4, 2, 4
        );

        testMoveAbsent(
            "Pinned rook cannot expose king",
            moves, 1, 4, 1, 3
        );
    }

    // King cannot move into check
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::King, Colour::White}); // d4
        board.setPiece(5, 3, {PieceType::Rook, Colour::Black}); // d6
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "King cannot move into rook check",
            moves, 3, 3, 4, 3
        );
    }

    // King can capture an unprotected enemy piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::King, Colour::White}); // d4
        board.setPiece(4, 4, {PieceType::Pawn, Colour::Black}); // e5
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        auto moves = generator.generateLegalMoves(board);

        testMove(
            "King can capture safe enemy piece",
            moves, 3, 3, 4, 4
        );
    }

    // King cannot capture a protected enemy piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::King, Colour::White}); // d4
        board.setPiece(4, 4, {PieceType::Pawn, Colour::Black}); // e5
        board.setPiece(5, 5, {PieceType::Bishop, Colour::Black}); // f6
        board.setPiece(7, 7, {PieceType::King, Colour::Black}); // h8

        auto moves = generator.generateLegalMoves(board);

        testMoveAbsent(
            "King cannot capture protected enemy piece",
            moves, 3, 3, 4, 4
        );
    }
}

int main() {
    MoveGenerator generator;

    testPawnMoves(generator);
    testEnPassantMoves(generator);
    testKnightMoves(generator);
    testBishopMoves(generator);
    testRookMoves(generator);
    testQueenMoves(generator);
    testKingMoves(generator);
    testCastlingMoves(generator);
    testPseudoLegalMoves(generator);
    testLegalMoves(generator);

    std::cout << "\nTests complete.\n";

    return 0;
}