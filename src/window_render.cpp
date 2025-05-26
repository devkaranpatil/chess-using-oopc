// Win32 GDI drawing for WindowChessGame: board, highlights, coordinates, side panel, pieces.
#ifdef _WIN32
#include "window_game.h"

#include <string>

using namespace std;

void WindowChessGame::paint(){
    PAINTSTRUCT ps;
    HDC hdc=BeginPaint(hwnd,&ps);
    draw(hdc);
    EndPaint(hwnd,&ps);
}

void WindowChessGame::draw(HDC hdc){
    SetBkMode(hdc,TRANSPARENT);
    HFONT titleFont=CreateFont(22,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Segoe UI");
    HFONT textFont=CreateFont(17,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Segoe UI");
    HFONT oldFont=(HFONT)SelectObject(hdc,textFont);

    HBRUSH bg=CreateSolidBrush(RGB(246,242,234));
    RECT full{0,0,WIN_W,WIN_H};
    FillRect(hdc,&full,bg);
    DeleteObject(bg);

    drawBoard(hdc);
    drawSidePanel(hdc,titleFont,textFont);

    SelectObject(hdc,oldFont);
    DeleteObject(titleFont);
    DeleteObject(textFont);
}

void WindowChessGame::drawBoard(HDC hdc){
    HBRUSH light=CreateSolidBrush(RGB(238,217,181));
    HBRUSH dark=CreateSolidBrush(RGB(132,91,61));
    HPEN gridPen=CreatePen(PS_SOLID,1,RGB(92,62,42));
    HPEN oldPen=(HPEN)SelectObject(hdc,gridPen);

    for(int dr=0;dr<8;dr++){
        for(int dc=0;dc<8;dc++){
            int r=viewBottom==WHITE?7-dr:dr;
            int c=viewBottom==WHITE?dc:7-dc;
            RECT sq{LEFT+dc*SQ,TOP+dr*SQ,LEFT+(dc+1)*SQ,TOP+(dr+1)*SQ};
            FillRect(hdc,&sq,((r+c)%2)?dark:light);
            Rectangle(hdc,sq.left,sq.top,sq.right,sq.bottom);
        }
    }

    drawHighlights(hdc);

    for(int r=0;r<8;r++){
        for(int c=0;c<8;c++){
            Piece* p=board.at(r,c);
            if(!p) continue;
            int x,y;
            squareToScreen(r,c,x,y);
            drawPiece(hdc,p,x,y);
        }
    }

    drawCoordinates(hdc);

    SelectObject(hdc,oldPen);
    DeleteObject(gridPen);
    DeleteObject(light);
    DeleteObject(dark);
}

void WindowChessGame::drawHighlights(HDC hdc){
    if(selectedR!=-1){
        int x,y;
        squareToScreen(selectedR,selectedC,x,y);
        HPEN pen=CreatePen(PS_SOLID,4,RGB(255,220,60));
        HBRUSH hollow=(HBRUSH)GetStockObject(HOLLOW_BRUSH);
        HPEN oldPen=(HPEN)SelectObject(hdc,pen);
        HBRUSH oldBrush=(HBRUSH)SelectObject(hdc,hollow);
        Rectangle(hdc,x+3,y+3,x+SQ-3,y+SQ-3);
        SelectObject(hdc,oldBrush);
        SelectObject(hdc,oldPen);
        DeleteObject(pen);
    }

