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

    grid[m.tr][m.tc] = m.promo!=EMPTY_P
                       ? makePiece(m.promo,col)
                       : move(grid[m.fr][m.fc]);
    grid[m.fr][m.fc].reset();

    if(m.castle){
        if(m.tc==6){ grid[m.tr][5]=move(grid[m.tr][7]); }
        else       { grid[m.tr][3]=move(grid[m.tr][0]); }
    }

    if(typ==KING){
        if(col==WHITE) wKm=true;
        else           bKm=true;
        if(m.castle){
            if(col==WHITE){ if(m.tc==6) wRh=true; else wRa=true; }
            else           { if(m.tc==6) bRh=true; else bRa=true; }
        }
    }
    if(typ==ROOK){
        if(col==WHITE&&m.fr==0&&m.fc==0) wRa=true;
        if(col==WHITE&&m.fr==0&&m.fc==7) wRh=true;
        if(col==BLACK&&m.fr==7&&m.fc==0) bRa=true;
        if(col==BLACK&&m.fr==7&&m.fc==7) bRh=true;
    }
    if(capturedType==ROOK){
        if(capturedColor==WHITE&&capR==0&&capC==0) wRa=true;
        if(capturedColor==WHITE&&capR==0&&capC==7) wRh=true;
        if(capturedColor==BLACK&&capR==7&&capC==0) bRa=true;
        if(capturedColor==BLACK&&capR==7&&capC==7) bRh=true;
    }
    if(typ==PAWN&&abs(m.tr-m.fr)==2){ epR=(m.fr+m.tr)/2; epC=m.fc; }

    turn=(Color)(-(int)turn);
}

bool Board::isAttacked(int r, int c, Color by) const {
    // Pawns
    for(int dc:{-1,1}){ int pr=r+(-(int)by),nc_=c+dc;
        if(inBounds(pr,nc_)){ auto* p=grid[pr][nc_].get();
            if(p&&p->getColor()==by&&p->getType()==PAWN) return true; } }
    // Knights
    static const int nd[8][2]={{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for(auto& d:nd){ int nr=r+d[0],nc_=c+d[1];
        if(inBounds(nr,nc_)){ auto* p=grid[nr][nc_].get();
            if(p&&p->getColor()==by&&p->getType()==KNIGHT) return true; } }
    // Diagonals (Bishop / Queen)
    for(auto d:initializer_list<pair<int,int>>{{1,1},{1,-1},{-1,1},{-1,-1}}){
        int dr=d.first, dc=d.second;
        int nr=r+dr,nc_=c+dc;
        while(inBounds(nr,nc_)){ auto* p=grid[nr][nc_].get(); if(p){
            if(p->getColor()==by&&(p->getType()==BISHOP||p->getType()==QUEEN)) return true;
            break; } nr+=dr; nc_+=dc; } }
    // Straights (Rook / Queen)
    for(auto d:initializer_list<pair<int,int>>{{1,0},{-1,0},{0,1},{0,-1}}){
        int dr=d.first, dc=d.second;
        int nr=r+dr,nc_=c+dc;
        while(inBounds(nr,nc_)){ auto* p=grid[nr][nc_].get(); if(p){
            if(p->getColor()==by&&(p->getType()==ROOK||p->getType()==QUEEN)) return true;
            break; } nr+=dr; nc_+=dc; } }
    // King
    for(auto d:initializer_list<pair<int,int>>
