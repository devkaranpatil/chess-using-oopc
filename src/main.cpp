#include "game.h"
#include "window_game.h"

int main(){
#ifdef _WIN32
    WindowChessGame game;
    return game.run();
#else
    Game game;
    game.run();
    return 0;
#endif
}
