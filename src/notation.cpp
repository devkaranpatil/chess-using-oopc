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

    Board nb(*this);
    nb.applyMove(m);
    if(nb.inCheck(opp)){
        s+=nb.legalMoves(opp).empty()?'#':'+';
    }
    return s;
}

string cleanNotation(string s){
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

string lowerText(string s){
    for(char& ch:s) ch=(char)tolower(static_cast<unsigned char>(ch));
    return s;
}

PieceType promotionFromChar(char ch){
    switch(tolower(static_cast<unsigned char>(ch))){
        case 'q': return QUEEN;
        case 'r': return ROOK;
        case 'b': return BISHOP;
        case 'n': return KNIGHT;
        default:  return EMPTY_P;
    }
}

bool coordinateMatches(const string& inp, const Move& lm, const Board& board){
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
