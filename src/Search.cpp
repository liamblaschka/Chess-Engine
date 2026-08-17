#include "Search.h"
#include "Game.h"
#include "Board.h"
#include "Move.h"
#include <array>
#include <vector>
#include <limits>

int Search::maximise(Game& game, int depth) {
    int best_score = std::numeric_limits<int>::min();

    std::vector<Move> moves = game.getLegalMoves();
    GameState game_state = game.getGameState(moves);
    if (game_state == GameState::Draw) {
        return DRAW_SCORE;
    }
    if (game_state == GameState::Checkmate) {
        return -CHECKMATE_SCORE;
    }

    if (depth == max_depth) {
        return evaluate(game.getBoard());
    }

    for (const Move& move : moves) {
        game.makeMove(move);

        int score = minimise(game, depth + 1);
        if (score > best_score) {
            best_score = score;
        }

        game.undoMove();
    }

    return best_score;
}

int Search::minimise(Game& game, int depth) {
    int best_score = std::numeric_limits<int>::max();

    std::vector<Move> moves = game.getLegalMoves();
    GameState game_state = game.getGameState(moves);
    if (game_state == GameState::Draw) {
        return DRAW_SCORE;
    }
    if (game_state == GameState::Checkmate) {
        return CHECKMATE_SCORE;
    }

    if (depth == max_depth) {
        return evaluate(game.getBoard());
    }

    for (const Move& move : moves) {
        game.makeMove(move);

        int score = maximise(game, depth + 1);
        if (score < best_score) {
            best_score = score;
        }

        game.undoMove();
    }

    return best_score;
}

Move Search::minimax(Game& game) {
    Move best_move;
    std::vector<Move> moves = game.getLegalMoves();
    if (game.getTurn() == Colour::White) {
        int best_score = std::numeric_limits<int>::min();
        for (const Move& move : moves) {
            game.makeMove(move);

            int score = minimise(game, 1);
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }

            game.undoMove();
        }
    } else {
        int best_score = std::numeric_limits<int>::max();
        for (const Move& move : moves) {
            game.makeMove(move);

            int score = maximise(game, 1);
            if (score < best_score) {
                best_score = score;
                best_move = move;
            }

            game.undoMove();
        }
    }

    return best_move;
}

int Search::pieceValue(PieceType piece_type) const {
    switch (piece_type) {
        case (PieceType::Pawn):
            return 100;
        case (PieceType::Knight):
            return 300;
        case (PieceType::Bishop):
            return 300;
        case (PieceType::Rook):
            return 500;
        case (PieceType::Queen):
            return 900;
        default:
            return 0;
    }
}

int Search::evaluate(const Board& board) const {
    int score = 0;
    for (const Piece& piece : board.getSquares()) {
        int value = pieceValue(piece.type);
        if (piece.colour == Colour::White) {
            score += value;
        } else {
            score -= value;
        }
    }

    return score;
}
