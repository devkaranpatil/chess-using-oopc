// Console (ASCII-art) rendering of the board.
#include "board.h"

#include <iostream>
#include <string>
#include <vector>

using namespace std;

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

