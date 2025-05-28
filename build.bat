@echo off
REM Builds bin\chess.exe with g++ (MinGW). Works from any directory.
setlocal
cd /d "%~dp0"
if not exist bin mkdir bin

g++ -std=c++14 -Wall -Wextra -O2 -Iinclude ^
    src\main.cpp src\board.cpp src\board_print.cpp src\evaluation.cpp src\notation.cpp ^
    src\piece.cpp src\pieces.cpp src\human_player.cpp src\ai_player.cpp src\rules.cpp ^
    src\game.cpp src\window_game.cpp src\window_render.cpp ^
    -static -lgdi32 -luser32 -o bin\chess.exe
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)
echo Built bin\chess.exe
