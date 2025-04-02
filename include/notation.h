#pragma once

#include <string>

#include "move.h"
#include "types.h"

class Board;

// Square / piece naming helpers.
char        fileChar(int c);
char        rankChar(int r);
std::string squareName(int r,int c);
char        pieceLetter(PieceType t);

// Input parsing helpers used by the console and window front ends.
std::string cleanNotation(std::string s);
std::string lowerText(std::string s);
PieceType   promotionFromChar(char ch);
bool        coordinateMatches(const std::string& inp, const Move& lm, const Board& board);
