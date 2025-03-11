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

vector<Move> Rook::getCandidates(const Board& b, int r, int c) const {
    vector<Move> mv;
    slide(mv,b,r,c,{{1,0},{-1,0},{0,1},{0,-1}});
    return mv;
}

vector<Move> Queen::getCandidates(const Board& b, int r, int c) const {
    vector<Move> mv;
    slide(mv,b,r,c,{{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}});
    return mv;
}

vector<Move> King::getCandidates(const Board& b, int r, int c) const {
    vector<Move> mv;
    for(auto d:initializer_list<pair<int,int>>
            {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}}){
        int dr=d.first, dc=d.second;
        int nr=r+dr, nc=c+dc;
        if(b.inBounds(nr,nc)){
            Piece* t=b.at(nr,nc);
            if(!t||t->getColor()!=color) mv.emplace_back(r,c,nr,nc);
        }
    }
    // Castling (path-clear check only; attacked-square check in legalMoves)
    if(color==WHITE && !b.wKingMoved() && r==0 && c==4){
        if(!b.wRookHMoved()&&!b.at(0,5)&&!b.at(0,6)&&
           b.at(0,7)&&b.at(0,7)->getColor()==WHITE&&b.at(0,7)->getType()==ROOK)
           { Move m(0,4,0,6); m.castle=true; mv.push_back(m); }
        if(!b.wRookAMoved()&&!b.at(0,3)&&!b.at(0,2)&&!b.at(0,1)&&
           b.at(0,0)&&b.at(0,0)->getColor()==WHITE&&b.at(0,0)->getType()==ROOK)
           { Move m(0,4,0,2); m.castle=true; mv.push_back(m); }
    } else if(color==BLACK && !b.bKingMoved() && r==7 && c==4){
        if(!b.bRookHMoved()&&!b.at(7,5)&&!b.at(7,6)&&
           b.at(7,7)&&b.at(7,7)->getColor()==BLACK&&b.at(7,7)->getType()==ROOK)
           { Move m(7,4,7,6); m.castle=true; mv.push_back(m); }
        if(!b.bRookAMoved()&&!b.at(7,3)&&!b.at(7,2)&&!b.at(7,1)&&
           b.at(7,0)&&b.at(7,0)->getColor()==BLACK&&b.at(7,0)->getType()==ROOK)
           { Move m(7,4,7,2); m.castle=true; mv.push_back(m); }
    }
    return mv;
}
