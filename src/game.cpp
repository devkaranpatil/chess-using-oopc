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

