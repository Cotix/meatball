#include "board.hpp"
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <immintrin.h>
#include <cstring>

// Array of last dir + tile type for next direction
Direction LAST_DIR[4][3] = {
    {Direction::EAST, Direction::NORTH, Direction::WEST}, // last to north
    {Direction::NORTH, Direction::EAST, Direction::SOUTH}, // last to east
    {Direction::WEST, Direction::SOUTH, Direction::EAST}, // last to south
    {Direction::SOUTH, Direction::WEST, Direction::NORTH}, // from west   
};

Board::Board() {
    for (int i = 0; i < 63; i++) {
        this->grid[i] = GridField::EMPTY;
    }
    this->turn = Color::BLUE;
    for (int i = 0; i < 3; i++) {
        this->bitboard[i] = 0ull;
    }
}

Board::Board(Board* other) {
    for (int i = 0; i < 63; i++) {
        this->grid[i] = other->grid[i];
    }
    this->turn = other->turn;
    for (int i = 0; i < 3; i++) {
        this->bitboard[i] = other->bitboard[i];
    }
}

int get_x(int position) {
    return position % BOARD_WIDTH;
}

int get_y(int position) {
    return position / BOARD_WIDTH;
}

int update_position(int position, Direction towards) {
    switch (towards)
    {
    case Direction::NORTH:
        if (get_y(position) == 0) return -1;
        return position - BOARD_WIDTH;
    case Direction::EAST:
        if (get_x(position) == BOARD_WIDTH - 1) return -3;
        return position + 1;
    case Direction::SOUTH:
        if (get_y(position) == BOARD_HEIGHT - 1) return -1;
        return position + BOARD_WIDTH;
    case Direction::WEST:
        if (get_x(position) == 0) return -2;
        return position - 1;
    }
    assert(false);
    return -1;
}

GridField Board::get_position(int position) {
    assert(position >= 00 && position < 63);
    return this->grid[position];
}

GridField Board::get_position(int x, int y) {
    return this->get_position(y * BOARD_WIDTH + x);
}

inline uint64_t nthset(uint64_t x, unsigned n)
{
    return _pdep_u64(1ULL << n, x);
}

int Board::free_position() {
    uint64_t free = ((1ull << 63) - 1) ^ (this->bitboard[0] | this->bitboard[1] | this->bitboard[2]);
    int bits = __builtin_popcountll(free);
    int skip = (unsigned int)rand() % bits;
    uint64_t move = nthset(free, skip);
    return __builtin_ctzll(move);
}

bool Board::game_finished() {
    return (this->bitboard[0] | this->bitboard[1] | this->bitboard[2]) == ((1ull << 63) - 1);
}

WalkResult Board::walk_line(int begin_pos, Direction begin_dir, int position, int* distance, Direction last, int* start_counter) {
    while (true) {
        assert(position >= -3 && position < 63);
        if (this->grid[position] == GridField::EMPTY) return WalkResult::NEUTRAL;
        switch (position) {
        case -1:
            return WalkResult::NEUTRAL;
        case -2:
            return WalkResult::BLUE;
        case -3:
            return WalkResult::RED;
        default:
            break;
        }
        if (position == begin_pos) (*start_counter)++;

        Direction dir = LAST_DIR[(int)last][(int)this->grid[position]];
        if (begin_pos == position && dir == begin_dir) return WalkResult::CYCLE; //TODO: Hardcode from dir as begin_dir instead of beginning last dir

        (*distance)++;
        assert(*distance <= 1000);
        position = update_position(position, dir);
        last = dir;
    }
}

int Board::make_move(Move move) {
    return this->make_move(GridField(move.move), move.position);
}

int Board::make_move(GridField move, int position) {
    assert(this->grid[position] == GridField::EMPTY);
    assert(move < GridField::EMPTY);
    assert((this->bitboard[(int)move] & (1ull << position)) == 0ull);
    this->grid[position] = move;
    this->bitboard[(int)move] |= 1ull << position;
    WalkResult result[4] = { WalkResult::NEUTRAL, WalkResult::NEUTRAL, WalkResult::NEUTRAL, WalkResult::NEUTRAL };
    int dist[4] = { 1, 1, 1, 1 };
    int counter = 0;
    switch (move) {
    case GridField::LEFT:
        result[0] = this->walk_line(position, Direction::NORTH, update_position(position, Direction::NORTH), &dist[0], Direction::NORTH, &counter);
        if (result[0] == WalkResult::CYCLE) {
            result[1] = WalkResult::CYCLE;
            if (counter >= 3) break;
        }
        else {
            result[1] = this->walk_line(position, Direction::WEST, update_position(position, Direction::WEST), &dist[1], Direction::WEST, &counter);
        }
        result[2] = this->walk_line(position, Direction::EAST, update_position(position, Direction::EAST), &dist[2], Direction::EAST, &counter);
        if (result[2] == WalkResult::CYCLE) {
            result[3] = WalkResult::CYCLE;
        }
        else {
            result[3] = this->walk_line(position, Direction::SOUTH, update_position(position, Direction::SOUTH), &dist[3], Direction::SOUTH, &counter);
        }
        break;

    case GridField::STRAIGHT:
        result[0] = this->walk_line(position, Direction::NORTH, update_position(position, Direction::NORTH), &dist[0], Direction::NORTH, &counter);
        if (result[0] == WalkResult::CYCLE) {
            result[1] = WalkResult::CYCLE;
            if (counter >= 3) break;
        }
        else {
            result[1] = this->walk_line(position, Direction::SOUTH, update_position(position, Direction::SOUTH), &dist[1], Direction::SOUTH, &counter);
        }
        result[2] = this->walk_line(position, Direction::EAST, update_position(position, Direction::EAST), &dist[2], Direction::EAST, &counter);
        if (result[2] == WalkResult::CYCLE) {
            result[3] = WalkResult::CYCLE;
        }
        else {
            result[3] = this->walk_line(position, Direction::WEST, update_position(position, Direction::WEST), &dist[3], Direction::WEST, &counter);
        }
        break;

    case GridField::RIGHT:
        result[0] = this->walk_line(position, Direction::NORTH, update_position(position, Direction::NORTH), &dist[0], Direction::NORTH, &counter);
        if (result[0] == WalkResult::CYCLE) {
            result[1] = WalkResult::CYCLE;
            if (counter >= 3) break;
        }
        else {
            result[1] = this->walk_line(position, Direction::EAST, update_position(position, Direction::EAST), &dist[1], Direction::EAST, &counter);
        }
        result[2] = this->walk_line(position, Direction::SOUTH, update_position(position, Direction::SOUTH), &dist[2], Direction::SOUTH, &counter);
        if (result[2] == WalkResult::CYCLE) {
            result[3] = WalkResult::CYCLE;
        }
        else {
            result[3] = this->walk_line(position, Direction::WEST, update_position(position, Direction::WEST), &dist[3], Direction::WEST, &counter);
        }
        break;

    default:
        assert(false);
        break;
    }
    Color last_turn = this->turn;
    this->turn = Color(((int)this->turn) ^ 1);
    int score = 0;
    if (result[0] == WalkResult::CYCLE || result[2] == WalkResult::CYCLE) score -= 5;
    for (int i = 0; i != 4; i += 2) {
        if (result[i] != WalkResult::NEUTRAL && result[i + 1] != WalkResult::NEUTRAL && result[i] != WalkResult::CYCLE) {
            if (result[i] == result[i + 1]) {
                score -= 3;
                continue;
            }

            score += result[i] == WalkResult(last_turn) ? dist[i] : dist[i + 1];
        }
    }
    return score;
}


bool Board::temp_move(GridField move, int position) {
    if (this->grid[position] != GridField::EMPTY) {
        return false;
    }
    assert(move < GridField::EMPTY);
    assert((this->bitboard[(int)move] & (1ull << position)) == 0ull);
    this->grid[position] = move;
    this->bitboard[(int)move] |= 1ull << position;
    this->turn = Color(((int)this->turn) ^ 1);
    return true;
}

void Board::temp_unmove(GridField move, int position) {
    assert(this->grid[position] != GridField::EMPTY);
    assert((this->bitboard[(int)move] & (1ull << position)) != 0ull);
    this->grid[position] = GridField::EMPTY;
    this->bitboard[(int)move] ^= 1ull << position;
    this->turn = Color(((int)this->turn) ^ 1);
}

uint64_t hash64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

uint64_t Board::hash() {
    return hash64(this->bitboard[0]) ^ hash64(this->bitboard[1]) ^ hash64(this->bitboard[2]);
}

bool Board::equals(uint64_t* other_bitboards) {
    return other_bitboards[0] == this->bitboard[0] && other_bitboards[1] == this->bitboard[1] && other_bitboards[2] == this->bitboard[2];
}

void Board::random_play(int* scores) {
    int ptr = 0;
    while (!this->game_finished()) {
        scores[ptr] += this->make_move(GridField(rand() % 3), this->free_position());
        ptr ^= 1;
    }
}


Move toMove(GridField move, int position) {
    Move res;
    res.move = (int)move;
    res.position = position;
    return res;
}