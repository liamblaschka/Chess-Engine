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

int main() {
    MoveGenerator generator;

    // ============================================================
    // PAWNS
    // ============================================================

    std::cout << "\n--- Pawn Tests ---\n";

    // White pawn: one and two squares forward
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(1, 4, {PieceType::Pawn, Colour::White}); // e2

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("White pawn one square",
                 moves, 1, 4, 2, 4); // e2 -> e3

        testMove("White pawn two squares",
                 moves, 1, 4, 3, 4); // e2 -> e4
    }

    // Black pawn: one and two squares forward
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(6, 4, {PieceType::Pawn, Colour::Black}); // e7

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Black pawn one square",
                 moves, 6, 4, 5, 4); // e7 -> e6

        testMove("Black pawn two squares",
                 moves, 6, 4, 4, 4); // e7 -> e5
    }

    // White pawn captures both directions
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White}); // e4

        board.setPiece(4, 3, {PieceType::Pawn, Colour::Black}); // d5
        board.setPiece(4, 5, {PieceType::Pawn, Colour::Black}); // f5

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("White pawn captures toward a-file",
                 moves, 3, 4, 4, 3); // e4 -> d5

        testMove("White pawn captures toward h-file",
                 moves, 3, 4, 4, 5); // e4 -> f5
    }

    // Black pawn captures both directions
    {
        Board board;
        board.clear();
        board.setTurn(Colour::Black);

        board.setPiece(4, 4, {PieceType::Pawn, Colour::Black}); // e5

        board.setPiece(3, 3, {PieceType::Pawn, Colour::White}); // d4
        board.setPiece(3, 5, {PieceType::Pawn, Colour::White}); // f4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Black pawn captures toward a-file",
                 moves, 4, 4, 3, 3); // e5 -> d4

        testMove("Black pawn captures toward h-file",
                 moves, 4, 4, 3, 5); // e5 -> f4
    }

    // Pawn cannot capture an empty square
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White}); // e4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot capture empty a-side diagonal",
                       moves, 3, 4, 4, 3);

        testMoveAbsent("Pawn cannot capture empty h-side diagonal",
                       moves, 3, 4, 4, 5);
    }

    // Pawn cannot move forward if blocked
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(1, 4, {PieceType::Pawn, Colour::White}); // e2
        board.setPiece(2, 4, {PieceType::Pawn, Colour::Black}); // e3

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot move forward when blocked",
                       moves, 1, 4, 2, 4);

        testMoveAbsent("Pawn cannot move two squares when blocked",
                       moves, 1, 4, 3, 4);
    }

    // Pawn cannot jump over a piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(1, 4, {PieceType::Pawn, Colour::White}); // e2
        board.setPiece(2, 4, {PieceType::Pawn, Colour::Black}); // e3

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot jump over blocking piece",
                       moves, 1, 4, 3, 4);
    }

    // Pawn cannot capture friendly piece
    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 4, {PieceType::Pawn, Colour::White}); // e4
        board.setPiece(4, 3, {PieceType::Pawn, Colour::White}); // d5
        board.setPiece(4, 5, {PieceType::Pawn, Colour::White}); // f5

        auto moves = generator.generatePseudoLegalMoves(board);

        testMoveAbsent("Pawn cannot capture friendly piece on a-file side",
                       moves, 3, 4, 4, 3);

        testMoveAbsent("Pawn cannot capture friendly piece on h-file side",
                       moves, 3, 4, 4, 5);
    }

    // ============================================================
    // KNIGHTS
    // ============================================================

    std::cout << "\n--- Knight Tests ---\n";

    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::Knight, Colour::White}); // d4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Knight up-right",
                 moves, 3, 3, 5, 4);

        testMove("Knight up-left",
                 moves, 3, 3, 5, 2);

        testMove("Knight down-right",
                 moves, 3, 3, 1, 4);

        testMove("Knight down-left",
                 moves, 3, 3, 1, 2);

        testMove("Knight right-up",
                 moves, 3, 3, 4, 5);

        testMove("Knight right-down",
                 moves, 3, 3, 2, 5);

        testMove("Knight left-up",
                 moves, 3, 3, 4, 1);

        testMove("Knight left-down",
                 moves, 3, 3, 2, 1);
    }

    // ============================================================
    // BISHOPS
    // ============================================================

    std::cout << "\n--- Bishop Tests ---\n";

    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::Bishop, Colour::White}); // d4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Bishop up-right",
                 moves, 3, 3, 4, 4);

        testMove("Bishop up-left",
                 moves, 3, 3, 4, 2);

        testMove("Bishop down-right",
                 moves, 3, 3, 2, 4);

        testMove("Bishop down-left",
                 moves, 3, 3, 2, 2);

        // Further along each diagonal
        testMove("Bishop two squares up-right",
                 moves, 3, 3, 5, 5);

        testMove("Bishop two squares down-left",
                 moves, 3, 3, 1, 1);
    }

    // ============================================================
    // ROOKS
    // ============================================================

    std::cout << "\n--- Rook Tests ---\n";

    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::Rook, Colour::White}); // d4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("Rook up",
                 moves, 3, 3, 4, 3);

        testMove("Rook down",
                 moves, 3, 3, 2, 3);

        testMove("Rook toward a-file",
                 moves, 3, 3, 3, 2);

        testMove("Rook toward h-file",
                 moves, 3, 3, 3, 4);

        testMove("Rook two squares up",
                 moves, 3, 3, 5, 3);

        testMove("Rook two squares toward h-file",
                 moves, 3, 3, 3, 5);
    }

    // ============================================================
    // QUEENS
    // ============================================================

    std::cout << "\n--- Queen Tests ---\n";

    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::Queen, Colour::White}); // d4

        auto moves = generator.generatePseudoLegalMoves(board);

        // Rook-like movement
        testMove("Queen up",
                 moves, 3, 3, 4, 3);

        testMove("Queen down",
                 moves, 3, 3, 2, 3);

        testMove("Queen toward a-file",
                 moves, 3, 3, 3, 2);

        testMove("Queen toward h-file",
                 moves, 3, 3, 3, 4);

        // Bishop-like movement
        testMove("Queen up-right",
                 moves, 3, 3, 4, 4);

        testMove("Queen up-left",
                 moves, 3, 3, 4, 2);

        testMove("Queen down-right",
                 moves, 3, 3, 2, 4);

        testMove("Queen down-left",
                 moves, 3, 3, 2, 2);
    }

    // ============================================================
    // KINGS
    // ============================================================

    std::cout << "\n--- King Tests ---\n";

    {
        Board board;
        board.clear();
        board.setTurn(Colour::White);

        board.setPiece(3, 3, {PieceType::King, Colour::White}); // d4

        auto moves = generator.generatePseudoLegalMoves(board);

        testMove("King up",
                 moves, 3, 3, 4, 3);

        testMove("King down",
                 moves, 3, 3, 2, 3);

        testMove("King toward a-file",
                 moves, 3, 3, 3, 2);

        testMove("King toward h-file",
                 moves, 3, 3, 3, 4);

        testMove("King up-right",
                 moves, 3, 3, 4, 4);

        testMove("King up-left",
                 moves, 3, 3, 4, 2);

        testMove("King down-right",
                 moves, 3, 3, 2, 4);

        testMove("King down-left",
                 moves, 3, 3, 2, 2);
    }

    std::cout << "\nTests complete.\n";

    return 0;
}