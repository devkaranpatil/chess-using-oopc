#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <vector>

#include "board.h"
#include "move.h"
#include "piece.h"
#include "types.h"

// Win32 GDI front end: setup happens in the console, then the board opens in a window.
// Game flow and input live in window_game.cpp; drawing lives in window_render.cpp.
class WindowChessGame {
    static constexpr int SQ=80;
    static constexpr int LEFT=44;
    static constexpr int TOP=44;
    static constexpr int BOARD=SQ*8;
    static constexpr int WIN_W=LEFT*2+BOARD+220;
    static constexpr int WIN_H=TOP*2+BOARD+92;

    Board board;
    HWND hwnd=nullptr;
    bool twoPlayer=false;
    bool gameOver=false;
    Color humanColor=WHITE;
    Color viewBottom=WHITE;
    int aiDepth=2;
    int selectedR=-1,selectedC=-1;
    std::vector<Move> selectedMoves;
    std::vector<Move> pendingPromotionMoves;
    std::string status="White to move.";

    static LRESULT CALLBACK StaticProc(HWND h, UINT msg, WPARAM wp, LPARAM lp);

    static std::string colorName(Color c);
    static int  readIntOrDefault(int fallback);
    static char readCharOrDefault(char fallback);
    static void showRules();

    // --- window_game.cpp: setup, window, input, game flow ---
    void    setup();
    bool    createWindow();
    LRESULT proc(UINT msg, WPARAM wp, LPARAM lp);
    bool    screenToSquare(int x,int y,int& r,int& c) const;
    void    squareToScreen(int r,int c,int& x,int& y) const;
    bool    isHumanTurn() const;
    void    handleClick(int x,int y);
    void    selectSquare(int r,int c);
    void    beginPromotionChoice(const std::vector<Move>& choices);
    void    handleKey(WPARAM key);
    void    applyMove(const Move& m);
    void    makeAIMove();
    void    updateGameState(const std::string& prefix);

    // --- window_render.cpp: GDI drawing ---
    void paint();
    void draw(HDC hdc);
    void drawBoard(HDC hdc);
    void drawHighlights(HDC hdc);
    void drawCoordinates(HDC hdc);
    void drawSidePanel(HDC hdc, HFONT titleFont, HFONT textFont);
    void drawPiece(HDC hdc, Piece* p, int x, int y);
    void drawBase(HDC hdc,int x,int y);
    void drawPawn(HDC hdc,int x,int y);
    void drawKnight(HDC hdc,int x,int y);
    void drawBishop(HDC hdc,int x,int y);
    void drawRook(HDC hdc,int x,int y);
    void drawQueen(HDC hdc,int x,int y);
    void drawKing(HDC hdc,int x,int y);

public:
    int run();
};
#endif
