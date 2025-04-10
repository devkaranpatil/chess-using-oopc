#pragma once

#include <string>

#include "player.h"

// Reads moves from stdin (SAN like "Nf3" or coordinates like "g1f3").
class HumanPlayer : public Player {
public:
    HumanPlayer(Color c, const std::string& n): Player(c, n) {}

    Move getMove(const Board& board) override;
};
