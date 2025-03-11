#pragma once

#include <memory>
#include <string>
#include <vector>

#include "move.h"
#include "piece.h"
#include "types.h"

class Board {
    std::unique_ptr<Piece> grid[8][8];
    Color turn;
    bool  wKm,bKm;          // king-moved flags
    bool  wRa,wRh,bRa,bRh; // rook-moved flags
    int   epR,epC;          // en-passant target square (-1 if none)

public:
    Board();
    Board(const Board&);
    Board& operator=(const Board&);

    Piece* at(int r,int c)      const { return grid[r][c].get(); }
    Color  getTurn()            const { return turn; }
    int    getEpR()             const { return epR;  }
    int    getEpC()             const { return epC;  }
    bool   wKingMoved()         const { return wKm;  }
    bool   bKingMoved()         const { return bKm;  }
    bool   wRookAMoved()        const { return wRa;  }
    bool   wRookHMoved()        const { return wRh;  }
    bool   bRookAMoved()        const { return bRa;  }
    bool   bRookHMoved()        const { return bRh;  }
    bool   inBounds(int r,int c)const { return r>=0&&r<8&&c>=0&&c<8; }

    void              init();                                     // board.cpp
    void              applyMove(const Move&);                     // board.cpp
    bool              isAttacked(int r,int c,Color by) const;     // board.cpp
    bool              inCheck(Color col)               const;     // board.cpp
    std::vector<Move> pseudoLegal(Color col)           const;     // board.cpp
    std::vector<Move> legalMoves(Color col)            const;     // board.cpp
    int               evaluate()                       const;     // evaluation.cpp
    std::string       notation(const Move&)            const;     // notation.cpp
    void              print(Color bottomColor=WHITE)   const;     // board_print.cpp

private:
    std::unique_ptr<Piece> makePiece(PieceType, Color) const;
};
