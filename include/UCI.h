#pragma once

#include "Game.h"
#include "Search.h"

#include <string>

class UCI {
private:
    Game game;
    Search search;

    void handleCommand(const std::string& command);
    void uci();
    void isReady();
    void newGame();
    void position(const std::string& command);
    void go();
    void stop();
    Move parseMove(const std::string& move_string);
    std::string moveToUCI(const Move& move) const;
    int squareFromUCI(const std::string& square) const;
    std::string squareToUCI(int square) const;
public:
    void run();
};