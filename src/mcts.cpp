#include "mcts.hpp"
#include "table.hpp"
#include <cmath>
#include <cassert>
#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <cstring>
#include <cstdio>

float C = 0.175;
float DIVIDER = 50;
float HEUR = 0.9;

float get_score(TableNode* parent, TableNode* node, int moveid) {
    if (node == 0 || node->played == 0) return 10 + ((float)parent->childHeuristic[moveid]) / 255.0;
    assert(parent->played > 0 && node->won > 0);
    float heuristic = ((float)parent->childHeuristic[moveid] * HEUR) / 255.0;
    return (((float)node->won / (float)node->played / DIVIDER) + C * sqrt(log((float)parent->played) / (float)node->played)) + heuristic / node->played;
}

void expand_tree(Board* board) {
    TableNode* history[189];
    int ptr = 0;
    int scores[2] = { 50, 50 };
    TableNode* current = get_node(board);

    while (current != 0 && !board->game_finished()) {
        history[ptr++] = current;
        // Find next move
        // We do this by looping through all the moves and keeping a list of best moves
        // Then we chose a random move from that list
        Move best_moves[189];
        float best_score = -1;
        int move_ptr = 0;
        for (int position = 0; position < 63; position++) {
            for (int move = 0; move < 3; move++) {
                if (!board->temp_move(GridField(move), position)) continue;
                float score = get_score(current, get_node(board), move * 63 + position);
                board->temp_unmove(GridField(move), position);

                if (score > best_score) {
                    best_moves[0] = toMove(GridField(move), position);
                    move_ptr = 1;
                    best_score = score;
                }
                else if (score == best_score) {
                    best_moves[move_ptr++] = toMove(GridField(move), position);
                }
            }
        }
        assert(move_ptr > 0);
        scores[ptr % 2] += board->make_move(best_moves[rand() % move_ptr]);
        current = get_node(board);
    }

    // Expand tree
    current = add_node(board);
    history[ptr++] = current;

    // Finish with random play
    int tmpscores[2] = { 0, 0 };
    board->random_play(tmpscores);
    scores[ptr % 2] += tmpscores[0];
    scores[(ptr % 2) ^ 1] += tmpscores[1];

    // Update tree
    for (int i = 0; i < ptr; i++) {
        history[i]->played++;
        history[i]->won += scores[i % 2];
    }
}

Move get_best_move(Board* board) {
    TableNode* node = get_node(board);
    assert(node != 0);
    Move best_move;
    uint32_t best_score = 0;
    int move_ptr = 0;
    for (int position = 0; position < 63; position++) {
        for (int move = 0; move < 3; move++) {
            if (!board->temp_move(GridField(move), position)) continue;;
            TableNode* next = get_node(board);
            uint32_t score = 0;
            if (next != 0) {
                score = next->played;
            }
            board->temp_unmove(GridField(move), position);

            if (score > best_score) {
                best_move = toMove(GridField(move), position);
                best_score = score;
            }
        }
    }


    return best_move;
}

uint32_t mcts(Board* board, uint64_t ns) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    uint64_t time_now = spec.tv_sec * 1000000000lu + spec.tv_nsec;
    uint64_t expiration_date = time_now + ns;
    uint32_t iterations = 0;
    while (time_now <= expiration_date)
    {
        clock_gettime(CLOCK_REALTIME, &spec);
        time_now = spec.tv_sec * 1000000000lu + spec.tv_nsec;
        Board copy = Board(board);
        expand_tree(&copy);
        iterations++;
    }
    return iterations;
}