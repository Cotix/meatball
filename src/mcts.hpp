#ifndef HEADER_MCTS
#define HEADER_MCTS

#include "board.hpp"

void expand_tree(Board* board);
Move get_best_move(Board* board);
uint32_t mcts(Board* board, uint64_t ns);
#endif