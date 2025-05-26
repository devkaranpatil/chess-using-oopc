// Win32 front end: console setup, window creation, mouse/keyboard input and game flow.
#ifdef _WIN32
#include "window_game.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>

#include "ai_player.h"
#include "notation.h"
#include "rules.h"

using namespace std;

LRESULT CALLBACK WindowChessGame::StaticProc(HWND h, UINT msg, WPARAM wp, LPARAM lp){
    WindowChessGame* self=nullptr;
    if(msg==WM_NCCREATE){
        auto* cs=reinterpret_cast<CREATESTRUCT*>(lp);
        self=reinterpret_cast<WindowChessGame*>(cs->lpCreateParams);
        SetWindowLongPtr(h,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(self));
        self->hwnd=h;
    } else {
        self=reinterpret_cast<WindowChessGame*>(GetWindowLongPtr(h,GWLP_USERDATA));
    }
    return self?self->proc(msg,wp,lp):DefWindowProc(h,msg,wp,lp);
}

string WindowChessGame::colorName(Color c){ return c==WHITE?"White":"Black"; }

int WindowChessGame::readIntOrDefault(int fallback){
    int value;
    if(cin>>value) return value;
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    return fallback;
}

char WindowChessGame::readCharOrDefault(char fallback){
    char value;
    if(cin>>value) return value;
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
    return fallback;
}

void WindowChessGame::showRules(){
    printGeneralRules();
}

void WindowChessGame::setup(){
    cout<<"==============================\n";
    cout<<"  Chess made using Oopc\n";
    cout<<"  Graphical board mode\n";
    cout<<"==============================\n\n";

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

    twoPlayer=(mode==2);
    if(twoPlayer){
        viewBottom=WHITE;
    } else {
        cout<<"Play as (w)hite or (b)lack? ";
        char ch=readCharOrDefault('w');
        humanColor=(tolower(static_cast<unsigned char>(ch))=='b')?BLACK:WHITE;
        viewBottom=humanColor;

        cout<<"Difficulty (1=easy  2=medium  3=hard): ";
        int diff=readIntOrDefault(1);
        diff=max(1,min(3,diff));
        aiDepth=diff+1;
    }

    board.init();
    status=twoPlayer?"White to move.":"Your turn.";
    if(!twoPlayer&&board.getTurn()!=humanColor) makeAIMove();
}

bool WindowChessGame::createWindow(){
    WNDCLASS wc{};
    wc.lpfnWndProc=StaticProc;
    wc.hInstance=GetModuleHandle(nullptr);
    wc.lpszClassName="OOPChessWindow";
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
    wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    if(!RegisterClass(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return false;

    hwnd=CreateWindow(
        wc.lpszClassName,
        "Chess made using Oopc - Board",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT,CW_USEDEFAULT,WIN_W,WIN_H,
        nullptr,nullptr,wc.hInstance,this
    );
    return hwnd!=nullptr;
}

LRESULT WindowChessGame::proc(UINT msg, WPARAM wp, LPARAM lp){
    switch(msg){
        case WM_PAINT: paint(); return 0;
        case WM_LBUTTONDOWN:
            handleClick(static_cast<int>(static_cast<short>(LOWORD(lp))),
                        static_cast<int>(static_cast<short>(HIWORD(lp))));
            return 0;
        case WM_KEYDOWN:
            handleKey(wp);
            return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd,msg,wp,lp);
}

bool WindowChessGame::screenToSquare(int x,int y,int& r,int& c) const {
    if(x<LEFT||x>=LEFT+BOARD||y<TOP||y>=TOP+BOARD) return false;
    int dc=(x-LEFT)/SQ;
    int dr=(y-TOP)/SQ;
    if(viewBottom==WHITE){
        r=7-dr;
        c=dc;
    } else {
        r=dr;
        c=7-dc;
    }
    return true;
}

void WindowChessGame::squareToScreen(int r,int c,int& x,int& y) const {
    int dc=viewBottom==WHITE?c:7-c;
    int dr=viewBottom==WHITE?7-r:r;
    x=LEFT+dc*SQ;
    y=TOP+dr*SQ;
}

bool WindowChessGame::isHumanTurn() const {
    return twoPlayer||board.getTurn()==humanColor;
}

void WindowChessGame::handleClick(int x,int y){
    if(gameOver||!isHumanTurn()) return;

    int r,c;
    if(!screenToSquare(x,y,r,c)) return;
    Piece* p=board.at(r,c);
    Color turn=board.getTurn();

    if(selectedR==-1){
        if(p&&p->getColor()==turn) selectSquare(r,c);
        return;
    }

    if(p&&p->getColor()==turn){
        selectSquare(r,c);
        return;
    }

    vector<Move> promotionChoices;
    for(const Move& m:selectedMoves){
        if(m.tr!=r||m.tc!=c) continue;
        if(m.promo!=EMPTY_P) promotionChoices.push_back(m);
        else {
            applyMove(m);
            return;
        }
    }

    if(!promotionChoices.empty()){
        beginPromotionChoice(promotionChoices);
        return;
    }

    status="Illegal move. Pick a highlighted square.";
    selectedR=selectedC=-1;
    selectedMoves.clear();
    pendingPromotionMoves.clear();
    InvalidateRect(hwnd,nullptr,TRUE);
}

void WindowChessGame::selectSquare(int r,int c){
    selectedR=r;
    selectedC=c;
    selectedMoves.clear();
    pendingPromotionMoves.clear();
    for(const Move& m:board.legalMoves(board.getTurn())){
        if(m.fr==r&&m.fc==c) selectedMoves.push_back(m);
    }
    status=selectedMoves.empty()?"That piece has no legal moves.":"Choose a highlighted destination.";
    InvalidateRect(hwnd,nullptr,TRUE);
}

void WindowChessGame::beginPromotionChoice(const vector<Move>& choices){
    pendingPromotionMoves=choices;
    status="Promote pawn: press Q, R, B, or N.";
    InvalidateRect(hwnd,nullptr,TRUE);
}

void WindowChessGame::handleKey(WPARAM key){
    if(pendingPromotionMoves.empty()) return;

    if(key==VK_ESCAPE){
        pendingPromotionMoves.clear();
        selectedR=selectedC=-1;
        selectedMoves.clear();
        status="Promotion cancelled.";
        InvalidateRect(hwnd,nullptr,TRUE);
        return;
    }

