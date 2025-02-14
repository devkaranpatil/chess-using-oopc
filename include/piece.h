#pragma once

#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

#include "move.h"
#include "types.h"

class Board; // forward declaration needed by Piece

class Piece {
protected:
    Color     color;
    PieceType type;
public:
    Piece(Color c, PieceType t): color(c), type(t) {}
    virtual ~Piece() = default;

    Color     getColor() const { return color; }
    PieceType getType()  const { return type;  }

    virtual char                    getChar()      const = 0;
    virtual std::vector<Move>       getCandidates(const Board&, int r, int c) const = 0;
    virtual std::unique_ptr<Piece>  clone()        const = 0;

protected:
    // Adds sliding moves (bishop/rook/queen) along each direction until blocked.
    void slide(std::vector<Move>& mv, const Board& b, int r, int c,
               std::initializer_list<std::pair<int,int>> dirs) const;
};
