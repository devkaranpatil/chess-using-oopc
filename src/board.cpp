// Core rules: setup, move application, attack detection and legal move generation.
#include "board.h"

#include <cstdlib>
#include <initializer_list>
#include <utility>

#include "pieces.h"

using namespace std;

Board::Board():turn(WHITE),wKm(false),bKm(false),
    wRa(false),wRh(false),bRa(false),bRh(false),epR(-1),epC(-1){}

Board::Board(const Board& o)
    :turn(o.turn),wKm(o.wKm),bKm(o.bKm),
     wRa(o.wRa),wRh(o.wRh),bRa(o.bRa),bRh(o.bRh),epR(o.epR),epC(o.epC){
    for(int r=0;r<8;r++) for(int c=0;c<8;c++)
        if(o.grid[r][c]) grid[r][c]=o.grid[r][c]->clone();
}

Board& Board::operator=(const Board& o){
    if(this==&o) return *this;
    turn=o.turn; wKm=o.wKm; bKm=o.bKm;
    wRa=o.wRa; wRh=o.wRh; bRa=o.bRa; bRh=o.bRh;
    epR=o.epR; epC=o.epC;
    for(int r=0;r<8;r++) for(int c=0;c<8;c++)
        grid[r][c]=o.grid[r][c]?o.grid[r][c]->clone():nullptr;
    return *this;
}

unique_ptr<Piece> Board::makePiece(PieceType t, Color c) const {
    switch(t){
        case PAWN:   return make_unique<Pawn>(c);
        case KNIGHT: return make_unique<Knight>(c);
        case BISHOP: return make_unique<Bishop>(c);
        case ROOK:   return make_unique<Rook>(c);
        case QUEEN:  return make_unique<Queen>(c);
        case KING:   return make_unique<King>(c);
        default:     return nullptr;
    }
}

void Board::init(){
    for(int r=0;r<8;r++) for(int c=0;c<8;c++) grid[r][c].reset();
    static const PieceType back[]={ROOK,KNIGHT,BISHOP,QUEEN,KING,BISHOP,KNIGHT,ROOK};
    for(int c=0;c<8;c++){
        grid[0][c]=makePiece(back[c],WHITE);
        grid[1][c]=make_unique<Pawn>(WHITE);
        grid[6][c]=make_unique<Pawn>(BLACK);
        grid[7][c]=makePiece(back[c],BLACK);
    }
    turn=WHITE; wKm=bKm=wRa=wRh=bRa=bRh=false; epR=epC=-1;
}

void Board::applyMove(const Move& m){
    Piece* p=grid[m.fr][m.fc].get();
    Color  col=p->getColor();
    PieceType typ=p->getType();
    int capR=m.ep?m.fr:m.tr;
    int capC=m.tc;
    Piece* captured=grid[capR][capC].get();
    Color capturedColor=captured?captured->getColor():NONE_C;
    PieceType capturedType=captured?captured->getType():EMPTY_P;
    epR=epC=-1;

    if(m.ep) grid[m.fr][m.tc].reset();

