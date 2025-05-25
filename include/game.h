#pragma once

#include <memory>
#include <string>

#include "board.h"
#include "player.h"
#include "types.h"

// Console front end (used on non-Windows builds).
class Game {
    Board                        board;
    std::unique_ptr<Player>      players[2]; // [0]=white  [1]=black
    Color                        viewBottom=WHITE;
    bool                         flipBoardByTurn=false;

    Player& current(){ return *players[board.getTurn()==WHITE?0:1]; }

    void        clearBadInput();
    char        readCharOrDefault(char fallback);
    int         readIntOrDefault(int fallback);
    std::string readStringOrDefault(const std::string& fallback);

    void printBanner();
    void showRules();
    void setupSinglePlayer();
    void setupTwoPlayers();

public:
    void setup();
    void run();
};
