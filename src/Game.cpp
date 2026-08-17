#include "Game.h"
#include "Move.h"
#include "MoveGenerator.h"
#include "Board.h"

void Game::trackPosition(const std::vector<Move>& legal_moves) {
    positions[board.getPositionKey(legal_moves)]++;
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
        return GameState::Stalemate;
    }

    if (board.isInsufficientMaterial()) {
        return GameState::Draw;
    }

    if (in_check) {
        return GameState::Check;
    }

    return GameState::Playing;
}
