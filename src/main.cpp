#include "board.hpp"
#include "mcts.hpp"
#include "table.hpp"
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath> 

void to_notation(char* out, GridField move, int position) {
    int x = position % BOARD_WIDTH;
    int y = position / BOARD_WIDTH;
    char tile;
    switch (move) {
    case GridField::LEFT:
        tile = 'l';
        break;
    case GridField::RIGHT:
        tile = 'r';
        break;
    case GridField::STRAIGHT:
        tile = 's';
        break;
    }
    sprintf(out, "%c%c%c", 'a' + y, 'a' + x, tile);
}

void from_notation(char* in, GridField* move, int* position) {
    int x = in[1] - 'a';
    int y = in[0] - 'a';
    *position = y * BOARD_WIDTH + x;
    switch (in[2]) {
    case 'l':
        *move = GridField::LEFT;
        break;
    case 'r':
        *move = GridField::RIGHT;
        break;
    case 's':
        *move = GridField::STRAIGHT;
        break;
    }
}

void benchmark_board() {
    //Played 500K games in 3207 ms, thats 9822 Kmoves/sec!
    int games = 100 * 1000;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    for (int i = 0; i != games; ++i) {
        Board board;
        while (!board.game_finished()) {
            board.make_move(GridField(rand() % 3), board.free_position());
        }
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t diff = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    printf("Played %iK games in %i ms, thats %i Kmoves/sec!\n", games / 1000, diff, games * 63 / diff);
}

void generate_data() {
    init_tree();
    while (true) {
        Board board;
        int moves_played = (rand() % 3) + 2;
        for (int i = 0; i < moves_played; i++) {
            board.make_move(GridField(rand() % 3), board.free_position());
        }
        clear_tree();
        for (int i = 0; i != 50; ++i) {
            mcts(&board, (uint64_t)120 * 1000000000);
            for (int move = 0; move < 3; move++) {
                for (int position = 0; position < 63; position++) {
                    printf(((board.bitboard[move] >> position) & 1) == 1 ? "1," : "0,");
                }
            }
            TableNode* root = get_node(&board);
            assert(root->played >= 2000);
            for (int move = 0; move < 3; move++) {
                for (int position = 0; position < 63; position++) {
                    if (!board.temp_move(GridField(move), position)) {
                        printf("0,");
                        continue;
                    }
                    TableNode* current = get_node(&board);
                    board.temp_unmove(GridField(move), position);
                    float value = sqrt(((float)current->played) / root->played);
                    if (value >= 1) value = 1;
                    printf("%f,", value);
                }
            }
            printf("\n");
            fflush(stdout);
            board.make_move(get_best_move(&board));
        }
    }
}

int main() {
    generate_data();
    init_tree();
    char buffer[32];
    Board board;
    GridField move;
    int position;

    // read first two moves
    for (int i = 0; i < 2; i++) {
        scanf("%s", buffer);
        from_notation(buffer, &move, &position);
        board.make_move(move, position);
    }
    scanf("%s", buffer);
    if (strcmp(buffer, "Start") != 0) {
        from_notation(buffer, &move, &position);
        board.make_move(move, position);
    }
    int score = 50;
    while (!board.game_finished()) {
        int iterations = mcts(&board, 29.5 * 1000000000 / 63 * 2);
        fprintf(stderr, "Did %i treewalks\n", iterations);
        Move best_move = get_best_move(&board);
        board.make_move(best_move);
        to_notation(buffer, GridField(best_move.move), best_move.position);
        printf("%s\n", buffer);
        fflush(stdout);

        scanf("%s", buffer);
        if (strcmp(buffer, "Quit") == 0) {
            return 0;
        }
        from_notation(buffer, &move, &position);
        score += board.make_move(move, position);
        fprintf(stderr, "Made move %s: %i\n", buffer, score);
        fflush(stderr);
    }

}