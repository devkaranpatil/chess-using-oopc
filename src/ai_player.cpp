#include "ai_player.h"

#include <algorithm>
#include <iostream>

using namespace std;

constexpr int AIPlayer::INF;

AIPlayer::AIPlayer(Color c, int d): Player(c,"AI"), depth(max(1,d)) {}

int AIPlayer::minimax(const Board& b, int d, int alpha, int beta, bool maxing) const {
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

Move AIPlayer::getMove(const Board& board) {
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
