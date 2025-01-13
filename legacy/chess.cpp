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

