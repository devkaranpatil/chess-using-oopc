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

    HBRUSH dot=CreateSolidBrush(RGB(42,142,74));
    HPEN pen=CreatePen(PS_SOLID,1,RGB(42,142,74));
    HBRUSH oldBrush=(HBRUSH)SelectObject(hdc,dot);
    HPEN oldPen=(HPEN)SelectObject(hdc,pen);
    for(const Move& m:selectedMoves){
        int x,y;
        squareToScreen(m.tr,m.tc,x,y);
        Ellipse(hdc,x+SQ/2-9,y+SQ/2-9,x+SQ/2+9,y+SQ/2+9);
    }
    SelectObject(hdc,oldBrush);
    SelectObject(hdc,oldPen);
    DeleteObject(dot);
    DeleteObject(pen);
}

void WindowChessGame::drawCoordinates(HDC hdc){
    SetTextColor(hdc,RGB(54,42,32));
    for(int dc=0;dc<8;dc++){
        int c=viewBottom==WHITE?dc:7-dc;
        string file(1,(char)('a'+c));
        RECT top{LEFT+dc*SQ,TOP-26,LEFT+(dc+1)*SQ,TOP-4};
        RECT bottom{LEFT+dc*SQ,TOP+BOARD+4,LEFT+(dc+1)*SQ,TOP+BOARD+26};
        DrawTextA(hdc,file.c_str(),-1,&top,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        DrawTextA(hdc,file.c_str(),-1,&bottom,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
    for(int dr=0;dr<8;dr++){
        int r=viewBottom==WHITE?7-dr:dr;
        string rank(1,(char)('1'+r));
        RECT left{LEFT-28,TOP+dr*SQ,LEFT-4,TOP+(dr+1)*SQ};
        RECT right{LEFT+BOARD+4,TOP+dr*SQ,LEFT+BOARD+28,TOP+(dr+1)*SQ};
        DrawTextA(hdc,rank.c_str(),-1,&left,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        DrawTextA(hdc,rank.c_str(),-1,&right,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
}

void WindowChessGame::drawSidePanel(HDC hdc, HFONT titleFont, HFONT textFont){
    int x=LEFT+BOARD+48;
    RECT title{x,TOP,x+185,TOP+56};
    HFONT old=(HFONT)SelectObject(hdc,titleFont);
    SetTextColor(hdc,RGB(34,30,26));
    DrawTextA(hdc,"Chess made\nusing Oopc",-1,&title,DT_LEFT|DT_WORDBREAK);
    SelectObject(hdc,textFont);

    RECT view{x,TOP+68,x+180,TOP+110};
    string viewText="View: "+colorName(viewBottom)+" bottom";
    DrawTextA(hdc,viewText.c_str(),-1,&view,DT_LEFT|DT_WORDBREAK);

    RECT turn{x,TOP+120,x+180,TOP+178};
    string turnText=gameOver?"Game over":"Turn: "+colorName(board.getTurn());
    DrawTextA(hdc,turnText.c_str(),-1,&turn,DT_LEFT|DT_WORDBREAK);

    RECT help{x,TOP+190,x+185,TOP+328};
    string helpText="Click a piece, then click a highlighted square.\n\nFor promotion, press Q, R, B, or N.";
    DrawTextA(hdc,helpText.c_str(),-1,&help,DT_LEFT|DT_WORDBREAK);

    RECT st{x,TOP+340,x+185,TOP+455};
    DrawTextA(hdc,status.c_str(),-1,&st,DT_LEFT|DT_WORDBREAK);

    SelectObject(hdc,old);
}

void WindowChessGame::drawPiece(HDC hdc, Piece* p, int x, int y){
    bool white=p->getColor()==WHITE;
    COLORREF fill=white?RGB(248,246,234):RGB(33,35,39);
    COLORREF edge=white?RGB(64,55,46):RGB(238,232,218);
    HBRUSH brush=CreateSolidBrush(fill);
    HPEN pen=CreatePen(PS_SOLID,2,edge);
    HBRUSH oldBrush=(HBRUSH)SelectObject(hdc,brush);
    HPEN oldPen=(HPEN)SelectObject(hdc,pen);

    switch(p->getType()){
        case PAWN:   drawPawn(hdc,x,y); break;
        case KNIGHT: drawKnight(hdc,x,y); break;
        case BISHOP: drawBishop(hdc,x,y); break;
        case ROOK:   drawRook(hdc,x,y); break;
        case QUEEN:  drawQueen(hdc,x,y); break;
        case KING:   drawKing(hdc,x,y); break;
        default: break;
    }

    SelectObject(hdc,oldBrush);
    SelectObject(hdc,oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void WindowChessGame::drawBase(HDC hdc,int x,int y){
    RoundRect(hdc,x+18,y+60,x+62,y+69,8,8);
    RoundRect(hdc,x+25,y+51,x+55,y+62,8,8);
}

void WindowChessGame::drawPawn(HDC hdc,int x,int y){
    Ellipse(hdc,x+29,y+15,x+51,y+37);
    RoundRect(hdc,x+32,y+33,x+48,y+55,8,8);
    drawBase(hdc,x,y);
}

void WindowChessGame::drawKnight(HDC hdc,int x,int y){
    POINT body[]={{24+x,58+y},{31+x,28+y},{45+x,15+y},{58+x,23+y},{50+x,32+y},{58+x,45+y},{44+x,43+y},{39+x,58+y}};
    Polygon(hdc,body,8);
    Ellipse(hdc,x+44,y+23,x+49,y+28);
    drawBase(hdc,x,y);
}

void WindowChessGame::drawBishop(HDC hdc,int x,int y){
    Ellipse(hdc,x+28,y+12,x+52,y+36);
    POINT body[]={{40+x,31+y},{55+x,54+y},{25+x,54+y}};
    Polygon(hdc,body,3);
    MoveToEx(hdc,x+45,y+18,nullptr);
    LineTo(hdc,x+34,y+31);
    drawBase(hdc,x,y);
}

void WindowChessGame::drawRook(HDC hdc,int x,int y){
    Rectangle(hdc,x+22,y+17,x+58,y+31);
    Rectangle(hdc,x+24,y+27,x+56,y+55);
    Rectangle(hdc,x+22,y+12,x+30,y+22);
    Rectangle(hdc,x+36,y+12,x+44,y+22);
    Rectangle(hdc,x+50,y+12,x+58,y+22);
    drawBase(hdc,x,y);
}

void WindowChessGame::drawQueen(HDC hdc,int x,int y){
    Ellipse(hdc,x+18,y+13,x+30,y+25);
    Ellipse(hdc,x+34,y+8,x+46,y+20);
    Ellipse(hdc,x+50,y+13,x+62,y+25);
    POINT crown[]={{22+x,27+y},{30+x,50+y},{50+x,50+y},{58+x,27+y},{46+x,36+y},{40+x,24+y},{34+x,36+y}};
    Polygon(hdc,crown,7);
    drawBase(hdc,x,y);
}

void WindowChessGame::drawKing(HDC hdc,int x,int y){
    MoveToEx(hdc,x+40,y+9,nullptr);
    LineTo(hdc,x+40,y+27);
    MoveToEx(hdc,x+32,y+17,nullptr);
    LineTo(hdc,x+48,y+17);
    Ellipse(hdc,x+27,y+26,x+53,y+50);
    Rectangle(hdc,x+31,y+42,x+49,y+57);
    drawBase(hdc,x,y);
}
#endif
