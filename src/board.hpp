#ifndef HEADER_BOARD
#define HEADER_BOARD

#include <cstdint>

#define BOARD_WIDTH 7
#define BOARD_HEIGHT 9

enum class GridField {
    LEFT, STRAIGHT, RIGHT, EMPTY
};

enum class WalkResult {
    BLUE = 0, RED = 1, NEUTRAL, CYCLE
};

enum class Direction {
    NORTH, EAST, SOUTH, WEST
};

enum class Color {
    BLUE = 0, RED = 1
};

struct GameResult {
    int score[2] = { 0, 0 };
};

struct Move {
    uint8_t move : 2;
    uint8_t position : 6;
};

Move toMove(GridField move, int position);

class Board {
private:
    GridField grid[63];
    WalkResult walk_line(int begin_pos, Direction begin_dir, int position, int* distance, Direction last);

public:
    Board();
    Board(Board* other);
    uint64_t bitboard[3];
    int make_move(GridField move, int position);
    bool temp_move(GridField move, int position);
    void temp_unmove(GridField move, int position);
    int make_move(Move move);
    GridField get_position(int position);
    GridField get_position(int x, int y);
    int free_position();
    bool game_finished();
    uint64_t hash();
    bool equals(uint64_t* other_bitboards);
    void random_play(int* scores);
    Color turn;
};

#endif