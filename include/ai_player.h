#pragma once

#include "player.h"

// Minimax search with alpha-beta pruning over Board::evaluate().
class AIPlayer : public Player {
    int depth;
    static constexpr int INF=1'000'000;

    int minimax(const Board& b, int d, int alpha, int beta, bool maxing) const;

public:
    AIPlayer(Color c, int d);

    Move getMove(const Board& board) override;
};
