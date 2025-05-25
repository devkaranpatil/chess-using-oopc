#include "game.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>

#include "ai_player.h"
#include "human_player.h"
#include "rules.h"

using namespace std;

void Game::clearBadInput(){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

char Game::readCharOrDefault(char fallback){
    char value;
    if(cin>>value) return value;
    clearBadInput();
    return fallback;
}

int Game::readIntOrDefault(int fallback){
    int value;
    if(cin>>value) return value;
    clearBadInput();
    return fallback;
}

string Game::readStringOrDefault(const string& fallback){
    string value;
    if(cin>>value) return value;
    clearBadInput();
    return fallback;
}

void Game::printBanner(){
    cout<<"==============================\n";
    cout<<"  Chess made using Oopc\n";
    cout<<"  Console board mode\n";
    cout<<"==============================\n\n";
}

void Game::showRules(){
    printGeneralRules();
}

void Game::setupSinglePlayer(){
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

void Game::setupTwoPlayers(){
    cout<<"White player name: "; string n1=readStringOrDefault("White");
    cout<<"Black player name: "; string n2=readStringOrDefault("Black");
    players[0]=make_unique<HumanPlayer>(WHITE,n1);
    players[1]=make_unique<HumanPlayer>(BLACK,n2);
    viewBottom=WHITE;
    flipBoardByTurn=true;
}

void Game::setup(){
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

void Game::run(){
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
