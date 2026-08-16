#include "Board.h"
#include "MoveGenerator.h"
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
        board.setPiece(1, 4, {PieceType::Pawn, Colour::White}); // e2

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("White pawn one square", moves, 1, 4, 2, 4);
        testMove("White pawn two squares", moves, 1, 4, 3, 4);
    }

    // Black pawn: one and two squares
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);
        board.setPiece(6, 4, {PieceType::Pawn, Colour::Black}); // e7

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Black pawn one square", moves, 6, 4, 5, 4);
        testMove("Black pawn two squares", moves, 6, 4, 4, 4);
    }

    // White pawn captures
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White}); // e4
        board.setPiece(4, 3, {PieceType::Pawn, Colour::Black}); // d5
        board.setPiece(4, 5, {PieceType::Pawn, Colour::Black}); // f5

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("White pawn captures left", moves, 3, 4, 4, 3);
        testMove("White pawn captures right", moves, 3, 4, 4, 5);
    }

    // Black pawn captures
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(4, 4, {PieceType::Pawn, Colour::Black}); // e5
        board.setPiece(3, 3, {PieceType::Pawn, Colour::White}); // d4
        board.setPiece(3, 5, {PieceType::Pawn, Colour::White}); // f4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Black pawn captures left", moves, 4, 4, 3, 3);
        testMove("Black pawn captures right", moves, 4, 4, 3, 5);
    }

    // Pawn cannot capture empty square
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);
        board.setPiece(3, 4, {PieceType::Pawn, Colour::White});

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot capture empty left", moves, 3, 4, 4, 3);
        testMoveAbsent("Pawn cannot capture empty right", moves, 3, 4, 4, 5);
    }

    // Pawn blocked
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(1, 4, {PieceType::Pawn, Colour::White});
        board.setPiece(2, 4, {PieceType::Pawn, Colour::Black});

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot move when blocked", moves, 1, 4, 2, 4);
        testMoveAbsent("Pawn cannot double move when blocked", moves, 1, 4, 3, 4);
    }

    // Pawn cannot capture friendly piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White});
        board.setPiece(4, 3, {PieceType::Pawn, Colour::White});
        board.setPiece(4, 5, {PieceType::Pawn, Colour::White});

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot capture friendly left", moves, 3, 4, 4, 3);
        testMoveAbsent("Pawn cannot capture friendly right", moves, 3, 4, 4, 5);
    }
}

void testEnPassantMoves(MoveGenerator& generator) {
    std::cout << "\n--- En Passant Tests ---\n";

    // White captures black pawn en passant to the left
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black}); // d7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d5
        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        auto moves = generator.generatePseudoLegalMoves(board);

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

        board.setPiece(6, 5, {PieceType::Pawn, Colour::Black}); // f7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: f7 -> f5
        board.makeMove(
            Move(6, 5, 4, 5, MoveType::Normal)
        );

        auto moves = generator.generatePseudoLegalMoves(board);

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

        board.setPiece(1, 3, {PieceType::Pawn, Colour::White}); // d2
        board.setPiece(3, 4, {PieceType::Pawn, Colour::Black}); // e4

        // White: d2 -> d4
        board.makeMove(
            Move(1, 3, 3, 3, MoveType::Normal)
        );

        auto moves = generator.generatePseudoLegalMoves(board);

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

        board.setPiece(1, 5, {PieceType::Pawn, Colour::White}); // f2
        board.setPiece(3, 4, {PieceType::Pawn, Colour::Black}); // e4

        // White: f2 -> f4
        board.makeMove(
            Move(1, 5, 3, 5, MoveType::Normal)
        );

        auto moves = generator.generatePseudoLegalMoves(board);

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

        board.setPiece(5, 3, {PieceType::Pawn, Colour::Black}); // d6
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d6
        board.makeMove(
            Move(6, 3, 5, 3, MoveType::Normal)
        );

        auto moves = generator.generatePseudoLegalMoves(board);

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

        // It is now White's turn.
        // The last move was a7 -> a6, not d7 -> d5.
        auto moves = generator.generatePseudoLegalMoves(board);

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

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black}); // d7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d5
        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        auto moves = generator.generatePseudoLegalMoves(board);

        bool found_en_passant = false;

        for (const Move& move : moves) {
            if (move.from == 4 * 8 + 4 &&
                move.to == 5 * 8 + 3) {

                if (move.type == MoveType::EnPassant) {
                    found_en_passant = true;
                }
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

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black}); // d7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d5
        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        // White: e5 -> d6 en passant
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

        board.setPiece(6, 3, {PieceType::Pawn, Colour::Black}); // d7
        board.setPiece(4, 4, {PieceType::Pawn, Colour::White}); // e5

        // Black: d7 -> d5
        board.makeMove(
            Move(6, 3, 4, 3, MoveType::Normal)
        );

        // White: e5 -> d6 en passant
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
    board.setPiece(3, 3, {PieceType::Knight, Colour::White}); // d4

    auto moves = generator.generatePseudoLegalMoves(board);

    testMove("Knight up-right", moves, 3, 3, 5, 4);
    testMove("Knight up-left", moves, 3, 3, 5, 2);
    testMove("Knight down-right", moves, 3, 3, 1, 4);
    testMove("Knight down-left", moves, 3, 3, 1, 2);
    testMove("Knight right-up", moves, 3, 3, 4, 5);
    testMove("Knight right-down", moves, 3, 3, 2, 5);
    testMove("Knight left-up", moves, 3, 3, 4, 1);
    testMove("Knight left-down", moves, 3, 3, 2, 1);

    // Friendly piece cannot be captured
    board.setPiece(5, 4, {PieceType::Pawn, Colour::White});

    moves = generator.generatePseudoLegalMoves(board);

    testMoveAbsent(
        "Knight cannot capture friendly piece",
        moves, 3, 3, 5, 4
    );

    // Enemy piece can be captured
    board.setPiece(5, 2, {PieceType::Pawn, Colour::Black});

    moves = generator.generatePseudoLegalMoves(board);

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
    board.setPiece(3, 3, {PieceType::Bishop, Colour::White}); // d4

    auto moves = generator.generatePseudoLegalMoves(board);

    testMove("Bishop up-right", moves, 3, 3, 4, 4);
    testMove("Bishop up-left", moves, 3, 3, 4, 2);
    testMove("Bishop down-right", moves, 3, 3, 2, 4);
    testMove("Bishop down-left", moves, 3, 3, 2, 2);

    testMove("Bishop two squares up-right", moves, 3, 3, 5, 5);
    testMove("Bishop two squares down-left", moves, 3, 3, 1, 1);

    // Blocked by friendly piece
    board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

    moves = generator.generatePseudoLegalMoves(board);

    testMoveAbsent(
        "Bishop cannot move through friendly piece",
        moves, 3, 3, 5, 5
    );

    // Can capture enemy piece but cannot move beyond it
    board.clear();
    board.setTurn(Colour::White);
    board.setPiece(3, 3, {PieceType::Bishop, Colour::White});
    board.setPiece(5, 5, {PieceType::Pawn, Colour::Black});

    moves = generator.generatePseudoLegalMoves(board);

    testMove("Bishop can capture enemy piece", moves, 3, 3, 5, 5);
    testMoveAbsent("Bishop cannot move through enemy piece", moves, 3, 3, 6, 6);
}

void testRookMoves(MoveGenerator& generator) {
    std::cout << "\n--- Rook Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);
    board.setPiece(3, 3, {PieceType::Rook, Colour::White}); // d4

    auto moves = generator.generatePseudoLegalMoves(board);

    testMove("Rook up", moves, 3, 3, 4, 3);
    testMove("Rook down", moves, 3, 3, 2, 3);
    testMove("Rook left", moves, 3, 3, 3, 2);
    testMove("Rook right", moves, 3, 3, 3, 4);

    testMove("Rook two squares up", moves, 3, 3, 5, 3);
    testMove("Rook two squares right", moves, 3, 3, 3, 5);

    // Blocked by friendly piece
    board.setPiece(4, 3, {PieceType::Pawn, Colour::White});

    moves = generator.generatePseudoLegalMoves(board);

    testMoveAbsent(
        "Rook cannot move through friendly piece",
        moves, 3, 3, 5, 3
    );

    // Capture enemy piece and stop
    board.clear();
    board.setTurn(Colour::White);
    board.setPiece(3, 3, {PieceType::Rook, Colour::White});
    board.setPiece(3, 5, {PieceType::Pawn, Colour::Black});

    moves = generator.generatePseudoLegalMoves(board);

    testMove("Rook can capture enemy piece", moves, 3, 3, 3, 5);
    testMoveAbsent("Rook cannot move through enemy piece", moves, 3, 3, 3, 6);
}

void testQueenMoves(MoveGenerator& generator) {
    std::cout << "\n--- Queen Tests ---\n";

    Board board;
    board.clear();
    board.setTurn(Colour::White);
    board.setPiece(3, 3, {PieceType::Queen, Colour::White}); // d4

    auto moves = generator.generatePseudoLegalMoves(board);

    // Rook-like movement
    testMove("Queen up", moves, 3, 3, 4, 3);
    testMove("Queen down", moves, 3, 3, 2, 3);
    testMove("Queen left", moves, 3, 3, 3, 2);
    testMove("Queen right", moves, 3, 3, 3, 4);

    // Bishop-like movement
    testMove("Queen up-right", moves, 3, 3, 4, 4);
    testMove("Queen up-left", moves, 3, 3, 4, 2);
    testMove("Queen down-right", moves, 3, 3, 2, 4);
    testMove("Queen down-left", moves, 3, 3, 2, 2);

    // Further movement
    testMove("Queen two squares up", moves, 3, 3, 5, 3);
    testMove("Queen two squares up-right", moves, 3, 3, 5, 5);

    // Blocked by friendly piece
    board.setPiece(4, 4, {PieceType::Pawn, Colour::White});

    moves = generator.generatePseudoLegalMoves(board);

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

    auto moves = generator.generatePseudoLegalMoves(board);

    testMove("King up", moves, 3, 3, 4, 3);
    testMove("King down", moves, 3, 3, 2, 3);
    testMove("King left", moves, 3, 3, 3, 2);
    testMove("King right", moves, 3, 3, 3, 4);

    testMove("King up-right", moves, 3, 3, 4, 4);
    testMove("King up-left", moves, 3, 3, 4, 2);
    testMove("King down-right", moves, 3, 3, 2, 4);
    testMove("King down-left", moves, 3, 3, 2, 2);

    // Cannot capture friendly piece
    board.setPiece(4, 3, {PieceType::Pawn, Colour::White});

    moves = generator.generatePseudoLegalMoves(board);

    testMoveAbsent(
        "King cannot capture friendly piece",
        moves, 3, 3, 4, 3
    );

    // Can capture enemy piece
    board.setPiece(2, 3, {PieceType::Pawn, Colour::Black});

    moves = generator.generatePseudoLegalMoves(board);

    testMove(
        "King can capture enemy piece",
        moves, 3, 3, 2, 3
    );
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
    testPseudoLegalMoves(generator);
    testLegalMoves(generator);

    std::cout << "\nTests complete.\n";

    return 0;
}