#include "piece.h"

#include "board.h"

using namespace std;

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
