#pragma once

#include <string>

#include "board.h"
#include "move.h"
#include "types.h"

class Player {
protected:
    Color       color;
    std::string name;
public:
    Player(Color c, const std::string& n): color(c), name(n) {}
    virtual ~Player() = default;
    Color              getColor() const { return color; }
    const std::string& getName()  const { return name;  }
    virtual Move       getMove(const Board& board) = 0;
};
