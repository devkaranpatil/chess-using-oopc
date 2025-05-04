#include "human_player.h"

#include <iostream>
#include <string>

#include "notation.h"

using namespace std;

Move HumanPlayer::getMove(const Board& board) {
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
