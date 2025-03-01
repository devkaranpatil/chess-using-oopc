#pragma once

#include <memory>
#include <vector>

#include "piece.h"

class Pawn : public Piece {
public:
    explicit Pawn(Color c): Piece(c,PAWN) {}
    char getChar() const override { return color==WHITE?'P':'p'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Pawn>(color); }
    std::vector<Move> getCandidates(const Board& b, int r, int c) const override;
};

class Knight : public Piece {
public:
    explicit Knight(Color c): Piece(c,KNIGHT) {}
    char getChar() const override { return color==WHITE?'N':'n'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Knight>(color); }
    std::vector<Move> getCandidates(const Board& b, int r, int c) const override;
};

class Bishop : public Piece {
public:
    explicit Bishop(Color c): Piece(c,BISHOP) {}
    char getChar() const override { return color==WHITE?'B':'b'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Bishop>(color); }
    std::vector<Move> getCandidates(const Board& b, int r, int c) const override;
};

class Rook : public Piece {
public:
    explicit Rook(Color c): Piece(c,ROOK) {}
    char getChar() const override { return color==WHITE?'R':'r'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Rook>(color); }
    std::vector<Move> getCandidates(const Board& b, int r, int c) const override;
};

class Queen : public Piece {
public:
    explicit Queen(Color c): Piece(c,QUEEN) {}
    char getChar() const override { return color==WHITE?'Q':'q'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<Queen>(color); }
    std::vector<Move> getCandidates(const Board& b, int r, int c) const override;
};

class King : public Piece {
public:
    explicit King(Color c): Piece(c,KING) {}
    char getChar() const override { return color==WHITE?'K':'k'; }
    std::unique_ptr<Piece> clone() const override { return std::make_unique<King>(color); }
    std::vector<Move> getCandidates(const Board& b, int r, int c) const override;
};
