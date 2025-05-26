cmake_minimum_required(VERSION 3.10)
project(ChessOOP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chess
    src/main.cpp
    src/board.cpp
    src/board_print.cpp
    src/evaluation.cpp
    src/notation.cpp
    src/piece.cpp
    src/pieces.cpp
    src/human_player.cpp
    src/ai_player.cpp
    src/rules.cpp
    src/game.cpp
    src/window_game.cpp
    src/window_render.cpp
)
target_include_directories(chess PRIVATE include)

if(WIN32)
    target_link_libraries(chess PRIVATE gdi32 user32)
endif()

if(MSVC)
    target_compile_options(chess PRIVATE /W4)
else()
    target_compile_options(chess PRIVATE -Wall -Wextra)
endif()
