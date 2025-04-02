// Algebraic notation output (Board::notation) and move-input parsing helpers.
#include "notation.h"

#include <algorithm>
#include <cctype>

#include "board.h"

using namespace std;

char fileChar(int c){ return (char)('a'+c); }
char rankChar(int r){ return (char)('1'+r); }
string squareName(int r,int c){
    string s;
    s+=fileChar(c);
    s+=rankChar(r);
    return s;
}

char pieceLetter(PieceType t){
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

