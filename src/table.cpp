#include "table.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>

TableBucket* TABLE_DATA;


inline __attribute__((always_inline)) uint32_t table_idx(uint64_t hash) {
    return (hash) % TABLE_SIZE;
}

TableNode* add_node(Board* board) {
    TableNode* existing = get_node(board);
    if (existing != 0)
        return existing;

    TableNode* nodes = TABLE_DATA[table_idx(board->hash())].bucket;
    for (int i = 0; i < BUCKET_COUNT; i++) {
        if (nodes[i].played == 0) {
            nodes[i].bitboard[0] = board->bitboard[0];
            nodes[i].bitboard[1] = board->bitboard[1];
            nodes[i].bitboard[2] = board->bitboard[2];
            return &nodes[i];
        }
    }

    int lowest_id = -1;
    uint64_t lowest_played = 9999999999999999L;

    for (int i = 0; i < BUCKET_COUNT; i++)
    {
        if (nodes[i].played <= lowest_played)
        {
            lowest_played = nodes[i].played;
            lowest_id = i;
        }
    }

    assert(lowest_id != -1);

    nodes[lowest_id].played = 0;
    nodes[lowest_id].won = 0;
    nodes[lowest_id].bitboard[0] = board->bitboard[0];
    nodes[lowest_id].bitboard[1] = board->bitboard[1];
    nodes[lowest_id].bitboard[2] = board->bitboard[2];

    return &nodes[lowest_id];
}

void init_tree()
{
    TABLE_DATA = (TableBucket*)malloc(TABLE_SIZE * sizeof(TableBucket));
    memset(TABLE_DATA, 0, TABLE_SIZE * sizeof(TableBucket));
}

void clear_tree()
{
    memset(TABLE_DATA, 0, TABLE_SIZE * sizeof(TableBucket));
}


void save_table(char* file_name)
{
    FILE* f = fopen(file_name, "wb");
    int res = fwrite(TABLE_DATA, sizeof(TableBucket) * TABLE_SIZE, 1, f);
    fclose(f);
}

void load_table(char* file_name)
{
    FILE* f = fopen(file_name, "rb");
    assert(f != 0);
    int res = fread(TABLE_DATA, sizeof(TableBucket), TABLE_SIZE, f);
    assert(res == TABLE_SIZE);
    fclose(f);
}

TableNode* get_node(Board* board) {
    TableNode* nodes = TABLE_DATA[table_idx(board->hash())].bucket;
    for (int i = 0; i < BUCKET_COUNT; i++) {
        if (board->equals(nodes[i].bitboard)) {
            return &nodes[i];
        }
    }
    return 0;
}
