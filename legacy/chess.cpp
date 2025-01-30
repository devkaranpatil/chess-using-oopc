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

vector<Move> Board::legalMoves(Color col) const {
    auto all=pseudoLegal(col); vector<Move> legal;
    Color opp=(Color)(-(int)col);
    for(auto& m:all){
        if(m.castle){
            if(inCheck(col)) continue;
            if(isAttacked(m.fr,(m.fc+m.tc)/2,opp)) continue;
            if(isAttacked(m.fr,m.tc,opp)) continue;
        }
        Board nb(*this); nb.applyMove(m);
        if(!nb.inCheck(col)) legal.push_back(m);
    }
    return legal;
}

static char fileChar(int c){ return (char)('a'+c); }
static char rankChar(int r){ return (char)('1'+r); }
static string squareName(int r,int c){
    string s;
    s+=fileChar(c);
    s+=rankChar(r);
    return s;
}

static char pieceLetter(PieceType t){
    switch(t){
        case KING:   return 'K';
        case QUEEN:  return 'Q';
        case ROOK:   return 'R';
        case BISHOP: return 'B';
        case KNIGHT: return 'N';
        default:     return '\0';
    }
}

string Board::notation(const Move& m) const {
    if(m.isResign()) return "resign";
    Piece* p=at(m.fr,m.fc);
    if(!p) return m.str();

    Color col=p->getColor();
    PieceType typ=p->getType();
    Color opp=(Color)(-(int)col);

    string s;
    if(m.castle){
        s=(m.tc==6)?"0-0":"0-0-0";
    } else {
        bool capture=m.ep||at(m.tr,m.tc);
        char letter=pieceLetter(typ);
        if(letter) s+=letter;

        if(typ!=PAWN){
            bool sameFile=false, sameRank=false, ambiguous=false;
            for(const auto& other:legalMoves(col)){
                if(other.fr==m.fr&&other.fc==m.fc&&other.tr==m.tr&&other.tc==m.tc) continue;
                if(other.tr!=m.tr||other.tc!=m.tc) continue;
                Piece* op=at(other.fr,other.fc);
                if(!op||op->getType()!=typ||op->getColor()!=col) continue;
                ambiguous=true;
                if(other.fc==m.fc) sameFile=true;
                if(other.fr==m.fr) sameRank=true;
            }
            if(ambiguous){
                if(!sameFile) s+=fileChar(m.fc);
                else if(!sameRank) s+=rankChar(m.fr);
                else {
                    s+=fileChar(m.fc);
                    s+=rankChar(m.fr);
                }
            }
        } else if(capture){
            s+=fileChar(m.fc);
        }

        if(capture) s+='x';
        s+=squareName(m.tr,m.tc);
        if(m.promo!=EMPTY_P){
            s+='=';
            s+=pieceLetter(m.promo);
        }
    }

    Board nb(*this);
    nb.applyMove(m);
    if(nb.inCheck(opp)){
        s+=nb.legalMoves(opp).empty()?'#':'+';
    }
    return s;
}

// Piece-square tables are written from White's point of view.
static const int PVAL[]={0,100,320,330,500,900,20000};
static const int PAWN_T[8][8]={{0,0,0,0,0,0,0,0},{50,50,50,50,50,50,50,50},{10,10,20,30,30,20,10,10},{5,5,10,25,25,10,5,5},{0,0,0,20,20,0,0,0},{5,-5,-10,0,0,-10,-5,5},{5,10,10,-20,-20,10,10,5},{0,0,0,0,0,0,0,0}};
static const int KNIG_T[8][8]={{-50,-40,-30,-30,-30,-30,-40,-50},{-40,-20,0,0,0,0,-20,-40},{-30,0,10,15,15,10,0,-30},{-30,5,15,20,20,15,5,-30},{-30,0,15,20,20,15,0,-30},{-30,5,10,15,15,10,5,-30},{-40,-20,0,5,5,0,-20,-40},{-50,-40,-30,-30,-30,-30,-40,-50}};
static const int BISH_T[8][8]={{-20,-10,-10,-10,-10,-10,-10,-20},{-10,0,0,0,0,0,0,-10},{-10,0,5,10,10,5,0,-10},{-10,5,5,10,10,5,5,-10},{-10,0,10,10,10,10,0,-10},{-10,10,10,10,10,10,10,-10},{-10,5,0,0,0,0,5,-10},{-20,-10,-10,-10,-10,-10,-10,-20}};
static const int ROOK_T[8][8]={{0,0,0,0,0,0,0,0},{5,10,10,10,10,10,10,5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{-5,0,0,0,0,0,0,-5},{0,0,0,5,5,0,0,0}};
static const int QUEE_T[8][8]={{-20,-10,-10,-5,-5,-10,-10,-20},{-10,0,0,0,0,0,0,-10},{-10,0,5,5,5,5,0,-10},{-5,0,5,5,5,5,0,-5},{0,0,5,5,5,5,0,-5},{-10,5,5,5,5,5,0,-10},{-10,0,5,0,0,0,0,-10},{-20,-10,-10,-5,-5,-10,-10,-20}};
static const int KING_T[8][8]={{-30,-40,-40,-50,-50,-40,-40,-30},{-30,-40,-40,-50,-50,-40,-40,-30},{-30,-40,-40,-50,-50,-40,-40,-30},{-30,-40,-40,-50,-50,-40,-40,-30},{-20,-30,-30,-40,-40,-30,-30,-20},{-10,-20,-20,-20,-20,-20,-20,-10},{20,20,0,0,0,0,20,20},{20,30,10,0,0,10,30,20}};

static int posScore(PieceType t, int r, int c, Color col){
    int ti=(col==WHITE)?(7-r):r;
    switch(t){
        case PAWN:   return PAWN_T[ti][c];
        case KNIGHT: return KNIG_T[ti][c];
        case BISHOP: return BISH_T[ti][c];
        case ROOK:   return ROOK_T[ti][c];
        case QUEEN:  return QUEE_T[ti][c];
        case KING:   return KING_T[ti][c];
        default:     return 0;
    }
}

int Board::evaluate() const {
    int score=0;
    for(int r=0;r<8;r++) for(int c=0;c<8;c++){
        auto* p=grid[r][c].get(); if(!p) continue;
        score+=(int)p->getColor()*(PVAL[p->getType()]+posScore(p->getType(),r,c,p->getColor()));
    }
    return score;
}

class Player {
protected:
    Color  color;
    string name;
public:
    Player(Color c, const string& n): color(c), name(n) {}
    virtual ~Player() = default;
    Color         getColor() const { return color; }
    const string& getName()  const { return name;  }
    virtual Move  getMove(const Board& board) = 0;
};

static string cleanNotation(string s){
    s.erase(remove_if(s.begin(),s.end(),[](unsigned char ch){
        return ch=='+'||ch=='#'||ch=='!'||ch=='?'||ch=='.';
    }),s.end());
    s.erase(remove_if(s.begin(),s.end(),[](unsigned char ch){
        return ch==' '||ch=='\t'||ch=='\r'||ch=='\n';
    }),s.end());
    if(!s.empty()&&s[0]>='1'&&s[0]<='9'){
        size_t pos=0;
        while(pos<s.size()&&isdigit(static_cast<unsigned char>(s[pos]))) pos++;
        s.erase(0,pos);
    }
    replace(s.begin(),s.end(),'O','0');
    replace(s.begin(),s.end(),'o','0');
    return s;
}

static string lowerText(string s){
    for(char& ch:s) ch=(char)tolower(static_cast<unsigned char>(ch));
    return s;
}

static PieceType promotionFromChar(char ch){
    switch(tolower(static_cast<unsigned char>(ch))){
        case 'q': return QUEEN;
        case 'r': return ROOK;
        case 'b': return BISHOP;
        case 'n': return KNIGHT;
        default:  return EMPTY_P;
    }
}

static bool coordinateMatches(const string& inp, const Move& lm, const Board& board){
    if(inp.size()<4) return false;
    string s=lowerText(inp);
    int fc=s[0]-'a', fr=s[1]-'1', tc=s[2]-'a', tr=s[3]-'1';
    if(lm.fr!=fr||lm.fc!=fc||lm.tr!=tr||lm.tc!=tc) return false;

    Piece* p=board.at(lm.fr,lm.fc);
    bool isPromo=p&&p->getType()==PAWN&&(lm.tr==0||lm.tr==7);
    if(!isPromo) return s.size()==4;

    size_t pieceIndex=(s.size()>=6&&s[4]=='=')?5:4;
    if(pieceIndex>=s.size()) return false;

    PieceType pp=promotionFromChar(s[pieceIndex]);
    return pp!=EMPTY_P&&lm.promo==pp;
}

class HumanPlayer : public Player {
public:
    HumanPlayer(Color c, const string& n): Player(c, n) {}

    Move getMove(const Board& board) override {
        auto legal=board.legalMoves(color);
        while(true){
            cout<<name<<"'s turn ("<<(color==WHITE?"White":"Black")<<"). Enter move: ";
            string inp;
            if(!(cin>>inp)){
                cout<<"\n  Input ended. Resigning.\n";
                return Move::resign();
            }
            if(inp=="resign") return Move::resign();

            string cleaned=cleanNotation(inp);
            for(auto& lm:legal){
                if(cleaned==cleanNotation(board.notation(lm))||coordinateMatches(inp,lm,board))
                    return lm;
            }
            cout<<"  Illegal move. Use e4, Nf3, Bxc6, 0-0, or e8=Q. Promotion needs Q, R, B, or N.\n";
        }
    }
};

class AIPlayer : public Player {
    int depth;
    static constexpr int INF=1'000'000;

    int minimax(const Board& b, int d, int alpha, int beta, bool maxing) const {
        if(d<=0) return b.evaluate();
        Color col=maxing?WHITE:BLACK;
        auto moves=b.legalMoves(col);
        if(moves.empty()) return b.inCheck(col)?(maxing?-INF:INF):0;

        if(maxing){
            int best=-INF;
            for(auto& m:moves){ Board nb(b); nb.applyMove(m);
                best=max(best,minimax(nb,d-1,alpha,beta,false));
                alpha=max(alpha,best); if(beta<=alpha) break; }
            return best;
        } else {
            int best=INF;
            for(auto& m:moves){ Board nb(b); nb.applyMove(m);
                best=min(best,minimax(nb,d-1,alpha,beta,true));
                beta=min(beta,best); if(beta<=alpha) break; }
            return best;
        }
    }

public:
    AIPlayer(Color c, int d): Player(c,"AI"), depth(max(1,d)) {}

    Move getMove(const Board& board) override {
        cout<<"  "<<(color==WHITE?"White":"Black")<<" AI is thinking...\n";
        auto moves=board.legalMoves(color);
        if(moves.empty()){
            cout<<"  AI has no legal moves.\n";
            return Move::resign();
        }
        Move best=moves[0]; int bv=(color==WHITE)?-INF:INF;
        for(auto& m:moves){
            Board nb(board); nb.applyMove(m);
            int v=minimax(nb,depth-1,-INF,INF,color!=WHITE);
            if(color==WHITE?v>bv:v<bv){ bv=v; best=m; }
        }
        cout<<"  AI plays: "<<board.notation(best)<<"\n\n";
        return best;
    }
};

static void printGeneralRules(){
    cout<<"\n=== General Chess Rules ===\n";
    cout<<"Goal: checkmate the enemy king so it is attacked and has no legal escape.\n";
    cout<<"White moves first. Players alternate one legal move at a time.\n";
    cout<<"You may not make a move that leaves your own king in check.\n";
    cout<<"Checkmate wins the game. Stalemate is a draw.\n";
    cout<<"A pawn that reaches the last rank must promote to a queen, rook, bishop, or knight.\n";
    cout<<"Castling and en-passant are allowed only when their normal chess conditions are met.\n\n";

    cout<<"Piece movement:\n";
    cout<<"  Pawn: moves forward one square, captures diagonally, and can move two squares from its start.\n";
    cout<<"  Knight: moves in an L shape and can jump over pieces.\n";
    cout<<"  Bishop: moves diagonally any number of squares.\n";
    cout<<"  Rook: moves horizontally or vertically any number of squares.\n";
    cout<<"  Queen: combines rook and bishop movement.\n";
    cout<<"  King: moves one square in any direction.\n\n";
}

class Game {
    Board                  board;
    unique_ptr<Player>     players[2]; // [0]=white  [1]=black
    Color                  viewBottom=WHITE;
    bool                   flipBoardByTurn=false;

    Player& current(){ return *players[board.getTurn()==WHITE?0:1]; }

    void clearBadInput(){
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    char readCharOrDefault(char fallback){
        char value;
        if(cin>>value) return value;
        clearBadInput();
        return fallback;
    }

    int readIntOrDefault(int fallback){
        int value;
        if(cin>>value) return value;
        clearBadInput();
        return fallback;
    }

    string readStringOrDefault(const string& fallback){
        string value;
        if(cin>>value) return value;
        clearBadInput();
        return fallback;
    }

    void printBanner(){
        cout<<"==============================\n";
        cout<<"  Chess made using Oopc\n";
        cout<<"  Console board mode\n";
        cout<<"==============================\n\n";
    }

    void showRules(){
        printGeneralRules();
    }

    void setupSinglePlayer(){
        cout<<"Play as (w)hite or (b)lack? ";
        char ch=readCharOrDefault('w');
        Color human=(tolower(static_cast<unsigned char>(ch))=='b')?BLACK:WHITE;

        cout<<"Your name: ";
        string nm=readStringOrDefault("Player");

        cout<<"Difficulty (1=easy  2=medium  3=hard): ";
        int diff=readIntOrDefault(1);
        diff=max(1,min(3,diff));
        int aiDepth=diff+1; // 2, 3, or 4 plies

        if(human==WHITE){
            players[0]=make_unique<HumanPlayer>(WHITE,nm);
            players[1]=make_unique<AIPlayer>(BLACK,aiDepth);
        } else {
            players[0]=make_unique<AIPlayer>(WHITE,aiDepth);
            players[1]=make_unique<HumanPlayer>(BLACK,nm);
        }
        viewBottom=human;
        flipBoardByTurn=false;
    }

    void setupTwoPlayers(){
        cout<<"White player name: "; string n1=readStringOrDefault("White");
        cout<<"Black player name: "; string n2=readStringOrDefault("Black");
        players[0]=make_unique<HumanPlayer>(WHITE,n1);
        players[1]=make_unique<HumanPlayer>(BLACK,n2);
        viewBottom=WHITE;
        flipBoardByTurn=true;
    }

public:
    void setup(){
        printBanner();
        int mode=1;
        while(true){
            cout<<"Select mode:\n";
            cout<<"  1. Single Player (vs AI)\n";
            cout<<"  2. Two Players (local)\n";
            cout<<"  3. Rules\n";
            cout<<"Choice: ";
            mode=readIntOrDefault(1);
            cout<<"\n";
            if(mode==3){
                showRules();
                continue;
            }
            break;
        }
        if(mode==2) setupTwoPlayers();
        else        setupSinglePlayer();
        board.init();
        cout<<"\nMove examples: e4 | Nf3 | Bxc6 | 0-0 | e8=Q | e7e8n | Type 'resign' to quit\n\n";
    }

    void run(){
        setup();
        while(true){
            Color cur=board.getTurn();
            if(flipBoardByTurn) viewBottom=cur;
            board.print(viewBottom);
            string who=(cur==WHITE)?"White":"Black";

            auto legal=board.legalMoves(cur);
            if(legal.empty()){
                if(board.inCheck(cur))
                    cout<<"  CHECKMATE!  "<<(cur==WHITE?"Black":"White")<<" wins!\n";
                else
                    cout<<"  STALEMATE!  It's a draw.\n";
                break;
            }
            if(board.inCheck(cur))
                cout<<"  *** "<<who<<" is in CHECK! ***\n";

            Move m=current().getMove(board);
            if(m.isResign()){
                cout<<"  "<<current().getName()<<" resigns!\n";
                break;
            }
            board.applyMove(m);
        }
        cout<<"\nThanks for playing!\n";
    }
};

#ifdef _WIN32
class WindowChessGame {
    static constexpr int SQ=80;
    static constexpr int LEFT=44;
    static constexpr int TOP=44;
    static constexpr int BOARD=SQ*8;
    static constexpr int WIN_W=LEFT*2+BOARD+220;
    static constexpr int WIN_H=TOP*2+BOARD+92;

    Board board;
    HWND hwnd=nullptr;
    bool twoPlayer=false;
    bool gameOver=false;
    Color humanColor=WHITE;
    Color viewBottom=WHITE;
    int aiDepth=2;
    int selectedR=-1,selectedC=-1;
    vector<Move> selectedMoves;
    vector<Move> pendingPromotionMoves;
    string status="White to move.";

    static LRESULT CALLBACK StaticProc(HWND h, UINT msg, WPARAM wp, LPARAM lp){
        WindowChessGame* self=nullptr;
        if(msg==WM_NCCREATE){
            auto* cs=reinterpret_cast<CREATESTRUCT*>(lp);
            self=reinterpret_cast<WindowChessGame*>(cs->lpCreateParams);
            SetWindowLongPtr(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));
            self->hwnd=h;
        } else {
            self=reinterpret_cast<WindowChessGame*>(GetWindowLongPtr(h,GWLP_USERDATA));
        }
        return self?self->proc(msg,wp,lp):DefWindowProc(h,msg,wp,lp);
    }

    static string colorName(Color c){ return c==WHITE?"White":"Black"; }

    static int readIntOrDefault(int fallback){
        int value;
        if(cin>>value) return value;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
        return fallback;
    }

    static char readCharOrDefault(char fallback){
        char value;
        if(cin>>value) return value;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
        return fallback;
    }

    static void showRules(){
        printGeneralRules();
    }

    void setup(){
        cout<<"==============================\n";
        cout<<"  Chess made using Oopc\n";
        cout<<"  Graphical board mode\n";
        cout<<"==============================\n\n";

        int mode=1;
        while(true){
            cout<<"Select mode:\n";
            cout<<"  1. Single Player (vs AI)\n";
            cout<<"  2. Two Players (local)\n";
            cout<<"  3. Rules\n";
            cout<<"Choice: ";
            mode=readIntOrDefault(1);
            cout<<"\n";
            if(mode==3){
                showRules();
                continue;
            }
            break;
        }

        twoPlayer=(mode==2);
        if(twoPlayer){
            viewBottom=WHITE;
        } else {
            cout<<"Play as (w)hite or (b)lack? ";
            char ch=readCharOrDefault('w');
            humanColor=(tolower(static_cast<unsigned char>(ch))=='b')?BLACK:WHITE;
            viewBottom=humanColor;

            cout<<"Difficulty (1=easy  2=medium  3=hard): ";
            int diff=readIntOrDefault(1);
            diff=max(1,min(3,diff));
            aiDepth=diff+1;
        }

        board.init();
        status=twoPlayer?"White to move.":"Your turn.";
        if(!twoPlayer&&board.getTurn()!=humanColor) makeAIMove();
    }

    bool createWindow(){
        WNDCLASS wc{};
        wc.lpfnWndProc=StaticProc;
        wc.hInstance=GetModuleHandle(nullptr);
        wc.lpszClassName="OOPChessWindow";
        wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
        wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
        if(!RegisterClass(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return false;

        hwnd=CreateWindow(
            wc.lpszClassName,
            "Chess made using Oopc - Board",
            WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
            CW_USEDEFAULT,CW_USEDEFAULT,WIN_W,WIN_H,
            nullptr,nullptr,wc.hInstance,this
        );
        return hwnd!=nullptr;
    }

    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp){
        switch(msg){
            case WM_PAINT: paint(); return 0;
            case WM_LBUTTONDOWN:
                handleClick(static_cast<int>(static_cast<short>(LOWORD(lp))),
                            static_cast<int>(static_cast<short>(HIWORD(lp))));
                return 0;
            case WM_KEYDOWN:
                handleKey(wp);
                return 0;
            case WM_DESTROY: PostQuitMessage(0); return 0;
        }
        return DefWindowProc(hwnd,msg,wp,lp);
    }

    bool screenToSquare(int x,int y,int& r,int& c) const {
        if(x<LEFT||x>=LEFT+BOARD||y<TOP||y>=TOP+BOARD) return false;
        int dc=(x-LEFT)/SQ;
        int dr=(y-TOP)/SQ;
        if(viewBottom==WHITE){
            r=7-dr;
            c=dc;
        } else {
            r=dr;
            c=7-dc;
        }
        return true;
    }

    void squareToScreen(int r,int c,int& x,int& y) const {
        int dc=viewBottom==WHITE?c:7-c;
        int dr=viewBottom==WHITE?7-r:r;
        x=LEFT+dc*SQ;
        y=TOP+dr*SQ;
    }

    bool isHumanTurn() const {
        return twoPlayer||board.getTurn()==humanColor;
    }

    void handleClick(int x,int y){
        if(gameOver||!isHumanTurn()) return;

        int r,c;
        if(!screenToSquare(x,y,r,c)) return;
        Piece* p=board.at(r,c);
        Color turn=board.getTurn();

        if(selectedR==-1){
            if(p&&p->getColor()==turn) selectSquare(r,c);
            return;
        }

        if(p&&p->getColor()==turn){
            selectSquare(r,c);
            return;
        }

        vector<Move> promotionChoices;
        for(const Move& m:selectedMoves){
            if(m.tr!=r||m.tc!=c) continue;
            if(m.promo!=EMPTY_P) promotionChoices.push_back(m);
            else {
                applyMove(m);
                return;
            }
        }

        if(!promotionChoices.empty()){
            beginPromotionChoice(promotionChoices);
            return;
        }

        status="Illegal move. Pick a highlighted square.";
        selectedR=selectedC=-1;
        selectedMoves.clear();
        pendingPromotionMoves.clear();
        InvalidateRect(hwnd,nullptr,TRUE);
    }

    void selectSquare(int r,int c){
        selectedR=r;
        selectedC=c;
        selectedMoves.clear();
        pendingPromotionMoves.clear();
        for(const Move& m:board.legalMoves(board.getTurn())){
            if(m.fr==r&&m.fc==c) selectedMoves.push_back(m);
        }
        status=selectedMoves.empty()?"That piece has no legal moves.":"Choose a highlighted destination.";
        InvalidateRect(hwnd,nullptr,TRUE);
    }

    void beginPromotionChoice(const vector<Move>& choices){
        pendingPromotionMoves=choices;
        status="Promote pawn: press Q, R, B, or N.";
        InvalidateRect(hwnd,nullptr,TRUE);
    }

    void handleKey(WPARAM key){
        if(pendingPromotionMoves.empty()) return;

        if(key==VK_ESCAPE){
            pendingPromotionMoves.clear();
            selectedR=selectedC=-1;
            selectedMoves.clear();
            status="Promotion cancelled.";
            InvalidateRect(hwnd,nullptr,TRUE);
            return;
        }

        PieceType choice=promotionFromChar(static_cast<char>(key));
        if(choice==EMPTY_P){
            status="Press Q, R, B, or N to promote.";
            InvalidateRect(hwnd,nullptr,TRUE);
            return;
        }

        for(const Move& m:pendingPromotionMoves){
            if(m.promo==choice){
                pendingPromotionMoves.clear();
                applyMove(m);
                return;
            }
        }
    }

    void applyMove(const Move& m){
        string played=board.notation(m);
        board.applyMove(m);
        selectedR=selectedC=-1;
        selectedMoves.clear();
        pendingPromotionMoves.clear();
        if(twoPlayer) viewBottom=board.getTurn();
        updateGameState(colorName((Color)(-(int)board.getTurn()))+" played "+played+".");

        if(!gameOver&&!twoPlayer&&board.getTurn()!=humanColor){
            makeAIMove();
        }
        InvalidateRect(hwnd,nullptr,TRUE);
    }

    void makeAIMove(){
        if(gameOver) return;
        AIPlayer ai(board.getTurn(),aiDepth);
        Move m=ai.getMove(board);
        if(m.isResign()){
            gameOver=true;
            status="AI has no legal moves.";
            return;
        }
        string played=board.notation(m);
        board.applyMove(m);
        status="AI played "+played+". Your turn.";
        updateGameState(status);
    }

    void updateGameState(const string& prefix){
        Color cur=board.getTurn();
        auto legal=board.legalMoves(cur);
        if(legal.empty()){
            gameOver=true;
            if(board.inCheck(cur))
                status="CHECKMATE! "+colorName((Color)(-(int)cur))+" wins.";
            else
                status="STALEMATE! Draw.";
            MessageBox(hwnd,status.c_str(),"Game over",MB_OK|MB_ICONINFORMATION);
            return;
        }

        status=prefix+" "+colorName(cur)+" to move.";
        if(board.inCheck(cur)) status+=" CHECK!";
    }

    void paint(){
        PAINTSTRUCT ps;
        HDC hdc=BeginPaint(hwnd,&ps);
        draw(hdc);
        EndPaint(hwnd,&ps);
    }

    void draw(HDC hdc){
        SetBkMode(hdc,TRANSPARENT);
        HFONT titleFont=CreateFont(22,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Segoe UI");
        HFONT textFont=CreateFont(17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Segoe UI");
        HFONT oldFont=(HFONT)SelectObject(hdc,textFont);

