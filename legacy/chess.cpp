#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <initializer_list>
#include <limits>
#include <utility>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
using namespace std;

enum Color     { NONE_C=0, WHITE=1, BLACK=-1 };
enum PieceType { EMPTY_P=0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

struct Move {
    int fr,fc,tr,tc;
    PieceType promo;
    bool castle,ep;

    Move():fr(0),fc(0),tr(0),tc(0),promo(EMPTY_P),castle(false),ep(false){}
    Move(int fr,int fc,int tr,int tc,
         PieceType p=EMPTY_P,bool c=false,bool e=false)
        :fr(fr),fc(fc),tr(tr),tc(tc),promo(p),castle(c),ep(e){}

    bool isResign() const { return fr==-1; }

    string str() const {
        if(isResign()) return "resign";
        string s;
        s+=(char)('a'+fc); s+=(char)('1'+fr);
        s+=(char)('a'+tc); s+=(char)('1'+tr);
        if(promo){ const char* pp=".pnbrqk"; s+=pp[promo]; }
        return s;
    }
    static Move resign(){ Move m; m.fr=m.fc=m.tr=m.tc=-1; return m; }
};

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

    virtual char               getChar()      const = 0;
    virtual vector<Move>       getCandidates(const Board&, int r, int c) const = 0;
    virtual unique_ptr<Piece>  clone()        const = 0;

protected:
    void slide(vector<Move>& mv, const Board& b, int r, int c,
               initializer_list<pair<int,int>> dirs) const;
};

class Board {
    unique_ptr<Piece> grid[8][8];
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

    void         init();
    void         applyMove(const Move&);
    bool         isAttacked(int r,int c,Color by) const;
    bool         inCheck(Color col)               const;
    vector<Move> pseudoLegal(Color col)           const;
    vector<Move> legalMoves(Color col)            const;
    int          evaluate()                       const;
    string       notation(const Move&)            const;
    void         print(Color bottomColor=WHITE)   const;

private:
    unique_ptr<Piece> makePiece(PieceType, Color) const;
};

class Pawn : public Piece {
public:
    explicit Pawn(Color c): Piece(c,PAWN) {}
    char getChar() const override { return color==WHITE?'P':'p'; }
    unique_ptr<Piece> clone() const override { return make_unique<Pawn>(color); }

    vector<Move> getCandidates(const Board& b, int r, int c) const override {
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
};

class Knight : public Piece {
public:
    explicit Knight(Color c): Piece(c,KNIGHT) {}
    char getChar() const override { return color==WHITE?'N':'n'; }
    unique_ptr<Piece> clone() const override { return make_unique<Knight>(color); }

    vector<Move> getCandidates(const Board& b, int r, int c) const override {
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
};

class Bishop : public Piece {
public:
    explicit Bishop(Color c): Piece(c,BISHOP) {}
    char getChar() const override { return color==WHITE?'B':'b'; }
    unique_ptr<Piece> clone() const override { return make_unique<Bishop>(color); }

    vector<Move> getCandidates(const Board& b, int r, int c) const override {
        vector<Move> mv;
        slide(mv,b,r,c,{{1,1},{1,-1},{-1,1},{-1,-1}});
        return mv;
    }
};

class Rook : public Piece {
public:
    explicit Rook(Color c): Piece(c,ROOK) {}
    char getChar() const override { return color==WHITE?'R':'r'; }
    unique_ptr<Piece> clone() const override { return make_unique<Rook>(color); }

    vector<Move> getCandidates(const Board& b, int r, int c) const override {
        vector<Move> mv;
        slide(mv,b,r,c,{{1,0},{-1,0},{0,1},{0,-1}});
        return mv;
    }
};

class Queen : public Piece {
public:
    explicit Queen(Color c): Piece(c,QUEEN) {}
    char getChar() const override { return color==WHITE?'Q':'q'; }
    unique_ptr<Piece> clone() const override { return make_unique<Queen>(color); }

    vector<Move> getCandidates(const Board& b, int r, int c) const override {
        vector<Move> mv;
        slide(mv,b,r,c,{{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}});
        return mv;
    }
};

class King : public Piece {
public:
    explicit King(Color c): Piece(c,KING) {}
    char getChar() const override { return color==WHITE?'K':'k'; }
    unique_ptr<Piece> clone() const override { return make_unique<King>(color); }

    vector<Move> getCandidates(const Board& b, int r, int c) const override {
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
};

void Piece::slide(vector<Move>& mv, const Board& b, int r, int c,
                  initializer_list<pair<int,int>> dirs) const {
    for(auto d:dirs){
        int dr=d.first, dc=d.second;
        int nr=r+dr, nc=c+dc;
        while(b.inBounds(nr,nc)){
            Piece* t=b.at(nr,nc);
            if(t && t->getColor()==color) break;
            mv.emplace_back(r,c,nr,nc);
            if(t) break;
            nr+=dr; nc+=dc;
        }
    }
}

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

static vector<string> pieceModel(Piece* p, bool darkSquare){
    if(!p){
        string fill=darkSquare?".......":"       ";
        return {fill,fill,fill,fill,fill};
    }

    char side=p->getColor()==WHITE?'W':'B';
    switch(p->getType()){
        case PAWN:   return {"       ","  (_)  ","   P   ","  /_\\  ","   "+string(1,side)+"   "};
        case KNIGHT: return {"  /)   "," /_/   ","   N   ","  /_\\  ","   "+string(1,side)+"   "};
        case BISHOP: return {"  /\\   ","  ()   ","   B   ","  /_\\  ","   "+string(1,side)+"   "};
        case ROOK:   return {"  [_]  "," [___] ","   R   ","  /_\\  ","   "+string(1,side)+"   "};
        case QUEEN:  return {"  _^_  "," (___) ","   Q   ","  /_\\  ","   "+string(1,side)+"   "};
        case KING:   return {"  _+_  "," (___) ","   K   ","  /_\\  ","   "+string(1,side)+"   "};
        default:     return {"???????","???????","???????","???????","???????"};
    }
}

void Board::print(Color bottomColor) const {
    cout<<"\n      ";
    for(int dc=0;dc<8;dc++){
        int c=bottomColor==WHITE?dc:7-dc;
        cout<<"   "<<(char)('a'+c)<<"    ";
    }
    cout<<"\n";

    string sep="    +";
    for(int c=0;c<8;c++) sep+="-------+";
    cout<<sep<<"\n";

    for(int dr=0;dr<8;dr++){
        int r=bottomColor==WHITE?7-dr:dr;
        vector<vector<string>> cells;
        for(int dc=0;dc<8;dc++){
            int c=bottomColor==WHITE?dc:7-dc;
            cells.push_back(pieceModel(grid[r][c].get(),(r+c)%2));
        }

        for(int line=0;line<5;line++){
            if(line==2) cout<<" "<<(r+1)<<"  |";
            else        cout<<"    |";
            for(int dc=0;dc<8;dc++) cout<<cells[dc][line]<<"|";
            if(line==2) cout<<" "<<(r+1);
            cout<<"\n";
        }
        cout<<sep<<"\n";
    }

    cout<<"      ";
    for(int dc=0;dc<8;dc++){
        int c=bottomColor==WHITE?dc:7-dc;
        cout<<"   "<<(char)('a'+c)<<"    ";
    }
    cout<<"\n";
    cout<<"    View: "<<(bottomColor==WHITE?"White":"Black")<<" at bottom. Pieces show P/N/B/R/Q/K and W/B color.\n\n";
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
            {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}}){
        int dr=d.first, dc=d.second;
        int nr=r+dr,nc_=c+dc;
        if(inBounds(nr,nc_)){ auto* p=grid[nr][nc_].get();
            if(p&&p->getColor()==by&&p->getType()==KING) return true; } }
    return false;
}

bool Board::inCheck(Color col) const {
    for(int r=0;r<8;r++) for(int c=0;c<8;c++){
        auto* p=grid[r][c].get();
        if(p&&p->getColor()==col&&p->getType()==KING)
            return isAttacked(r,c,(Color)(-(int)col));
    }
    return false;
}

vector<Move> Board::pseudoLegal(Color col) const {
    vector<Move> mv;
    for(int r=0;r<8;r++) for(int c=0;c<8;c++){
        auto* p=grid[r][c].get();
        if(p&&p->getColor()==col){
            auto pm=p->getCandidates(*this,r,c);
            mv.insert(mv.end(),pm.begin(),pm.end());
        }
    }
    return mv;
}

