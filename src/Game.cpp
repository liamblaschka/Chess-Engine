#include "Game.h"
#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"

GameState Game::getGameState(const std::vector<Move>& legal_moves) const {
    Colour turn = board.getTurn();
    bool in_check = board.isKingInCheck(turn);
    if (legal_moves.empty()) {
        if (in_check) {
            return GameState::Checkmate;
        }
        return GameState::Stalemate;
    }

    if (in_check) {
        return GameState::Check;
    }

    return GameState::Playing;
}
