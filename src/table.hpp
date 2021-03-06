#ifndef HEADER_TABLE
#define HEADER_TABLE

#include <cstdint>
#include "board.hpp"

#define TABLE_SIZE 997111
#define BUCKET_COUNT 8

struct TableNode {
    uint64_t bitboard[3];
    uint32_t played, won, lost;
};
typedef struct TableNode TableNode;

struct TableBucket
{
    TableNode bucket[BUCKET_COUNT];
};

typedef struct TableBucket TableBucket;

TableNode* add_node(Board* board);

void init_tree();

void clear_tree();

TableNode* get_node(Board* board);

void save_table(char* file_name);

void load_table(char* file_name);

#endif