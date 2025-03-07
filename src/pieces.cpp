#include "pieces.h"

#include "board.h"

using namespace std;

vector<Move> Pawn::getCandidates(const Board& b, int r, int c) const {
    vector<Move> mv;
    int dir=(int)color;                         // +1 white / -1 black
    int startR=(color==WHITE)?1:6;
    int promoR=(color==WHITE)?7:0;

    // One square forward
    if(b.inBounds(r+dir,c) && !b.at(r+dir,c)){
        if(r+dir==promoR)
            for(PieceType pp:{QUEEN,ROOK,BISHOP,KNIGHT})
                mv.emplace_back(r,c,r+dir,c,pp);
        else {
            mv.emplace_back(r,c,r+dir,c);
            // Two squares from start
            if(r==startR && !b.at(r+2*dir,c))
                mv.emplace_back(r,c,r+2*dir,c);
        }
    }
    // Diagonal captures + en-passant
    for(int dc:{-1,1}){
        int nr=r+dir, nc=c+dc;
        if(!b.inBounds(nr,nc)) continue;
        Piece* tgt=b.at(nr,nc);
        if(tgt && tgt->getColor()==(Color)(-color)){
            if(nr==promoR)
                for(PieceType pp:{QUEEN,ROOK,BISHOP,KNIGHT})
                    mv.emplace_back(r,c,nr,nc,pp);
            else
                mv.emplace_back(r,c,nr,nc);
        } else if(nr==b.getEpR() && nc==b.getEpC()){
            Move m(r,c,nr,nc); m.ep=true; mv.push_back(m);
        }
    }
    return mv;
}

vector<Move> Knight::getCandidates(const Board& b, int r, int c) const {
    vector<Move> mv;
    static const int d[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for(auto& dd:d){
        int nr=r+dd[0], nc=c+dd[1];
        if(b.inBounds(nr,nc)){
            Piece* t=b.at(nr,nc);
            if(!t||t->getColor()!=color) mv.emplace_back(r,c,nr,nc);
        }
    }
    return mv;
}

vector<Move> Bishop::getCandidates(const Board& b, int r, int c) const {
    vector<Move> mv;
    slide(mv,b,r,c,{{1,1},{1,-1},{-1,1},{-1,-1}});
    return mv;
}

