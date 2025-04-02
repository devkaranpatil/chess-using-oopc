#include "rules.h"

#include <iostream>

using namespace std;

void printGeneralRules(){
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
