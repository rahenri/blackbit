#ifndef HASH_HH
#define HASH_HH

#include "random.hh"
#include "board.hh"
#include <algorithm>

/* configurable params */

const size_t HASH_SIZE = 393241;
const size_t CLUSTER_SIZE = 8;



int32_t hash_hit, hash_miss, insert_count, hash_drops;

struct hash_slot {
    uint64_t hash_key;
    int32_t lower_bound;
    int32_t upper_bound;
    int32_t depth;
    Move move;

    /* debug */
    /*board_code code;
    uint64_t order;
    int alfa, beta;*/
};


/*
struct eval_hash_slot {
    uint64_t hash_key;
    int32_t score;
};
*/

static const size_t prime_list[] = {
    53ul,         97ul,         193ul,        389ul,       769ul,
    1543ul,       3079ul,       6151ul,       12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,     393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,    12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul,  402653189ul, 805306457ul,
    1610612741ul, 3221225473ul, 4294967291ul
};

hash_slot hash_table[2][HASH_SIZE][CLUSTER_SIZE];

void init_hash() {
    for(int turn=0;turn<2;++turn) {
        for(uint32_t i=0;i<HASH_SIZE;++i) {
            for(uint32_t j=0;j<CLUSTER_SIZE;++j) {
                hash_table[turn][i][j].hash_key = i+1;
            }
        }
    }
}

/*
eval_hash_slot* eval_hash_find(uint64_t hash_key, int turn) {
    eval_hash_slot* cand = eval_hash_table[turn] + hash_key % HASH_SIZE;
    if(cand->hash_key == hash_key) {
        return cand;
    } else {
        return 0;
    }
}

void eval_hash_insert(uint64_t hash_key, int turn, int score) {
    eval_hash_slot* cand = eval_hash_table[turn] + hash_key % HASH_SIZE;
    cand->score = score;
    cand->hash_key = hash_key;
}
*/

hash_slot* get_cluster(const Board& board) {
    return hash_table[board.turn][board.hash_key % HASH_SIZE];
}


hash_slot* find_key(const Board& board, hash_slot* cluster) {
    for(uint32_t i=0; i<CLUSTER_SIZE; ++i) {
        if(cluster[i].hash_key == board.hash_key) {
            for(int j=i;j>0;--j) {
                std::swap(cluster[j-1], cluster[j]);
            }
            return cluster;
        }
    }

    return 0;
}

hash_slot* hash_find(const Board& board) {
    hash_slot* cluster = get_cluster(board);

    return find_key(board, cluster);
}

void hash_insert(const Board& board, int depth, int lower_bound, int upper_bound, Move move) {
    size_t p = (board.hash_key) % HASH_SIZE;
    hash_slot* cand = hash_find(board);

    if(cand == 0) {
        hash_slot* cluster = get_cluster(board);
        for(int i=CLUSTER_SIZE-1;i>0;--i) {
            std::swap(cluster[i], cluster[i-1]);
        }
        cand = cluster;
    }

    if(cand->hash_key == board.hash_key) {
        if(cand->depth > depth) {
            return;
        } else if(cand->depth == depth) {
            lower_bound = std::max(lower_bound, cand->lower_bound);
            upper_bound = std::min(upper_bound, cand->upper_bound);
        }
    } else if(cand->hash_key != p+1) {
        hash_drops ++;
    }

    cand->hash_key = board.hash_key;
    cand->lower_bound = lower_bound;
    cand->upper_bound = upper_bound;
    cand->depth = depth;
    cand->move = move;
}

#endif
