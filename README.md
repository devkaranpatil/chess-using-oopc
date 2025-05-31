Chess using OOPC

Object-oriented chess in C++14: full move rules (castling, en passant, promotion, check/checkmate/stalemate),
a minimax + alpha-beta AI with three difficulty levels, and a Win32 graphical board
(console ASCII board on non-Windows systems).

## Project layout

```
Chess/
├── include/
│   ├── types.h          Color and PieceType enums
│   ├── move.h           Move struct
│   ├── piece.h          Abstract Piece base class (+ sliding helper)
│   ├── pieces.h         Pawn, Knight, Bishop, Rook, Queen, King
│   ├── board.h          Board state and rules API
│   ├── notation.h       Algebraic notation + move-input parsing helpers
│   ├── player.h         Abstract Player
│   ├── human_player.h   Console move input
│   ├── ai_player.h      Minimax AI
│   ├── rules.h          Rules text
│   ├── game.h           Console game loop (non-Windows)
│   └── window_game.h    Win32 graphical game (Windows)
├── src/
│   ├── main.cpp
│   ├── board.cpp         Setup, applyMove, attack detection, legal moves
│   ├── board_print.cpp   ASCII board rendering
│   ├── evaluation.cpp    Material + piece-square tables
│   ├── notation.cpp      Board::notation and input parsing
│   ├── piece.cpp / pieces.cpp
│   ├── human_player.cpp / ai_player.cpp
│   ├── rules.cpp / game.cpp
│   ├── window_game.cpp   Win32 setup, input and game flow
│   └── window_render.cpp Win32 GDI drawing
├── legacy/              Original single-file version (chess.cpp) + old .exe
├── build.bat
└── CMakeLists.txt
```

## Build

**Windows + MinGW (g++):** double-click `build.bat`, or run it from a terminal. Output: `bin\chess.exe`.

**CMake (any platform):**

```
cmake -S . -B build
cmake --build build
```

## Play

Choose a mode in the console, then play on the board window (Windows) or by typing moves (other systems).
Moves can be typed as `e4`, `Nf3`, `Bxc6`, `0-0`, `e8=Q`, or coordinates such as `e7e8n`. Type `resign` to quit.
