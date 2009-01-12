#ifndef HASH_HH
#define HASH_HH

#include "random.hh"
#include "board.hh"
#include <algorithm>

/* configurable params */

struct hash_slot {
    uint64_t hash_key;
    int32_t lower_bound;
    int32_t upper_bound;
    int32_t depth;
    Move move;
};

const size_t BUCKET_SIZE = 4;

struct hash_bucket {
    hash_slot slot[BUCKET_SIZE];
};

size_t hash_size = 0;

hash_bucket* hash_table[2];

bool is_prime(size_t N) {
    if(N==2) return true;
    if(N%2==0) return false;
    for(size_t i=3; i*i<=N; i+=2) {
        if(N%i==0) return false;
    }
    return true;
}

size_t next_prime(size_t N) {
    while(not is_prime(N)) ++N;
    return N;
}

void hash_set_size(size_t size) {
    hash_size = next_prime(size/sizeof(hash_bucket)/2);

    for(int turn=0;turn<2;++turn) {
        if(hash_table[turn]) delete hash_table[turn];
        hash_table[turn] = new hash_bucket[hash_size];
        for(uint32_t i=0;i<hash_size;++i) {
            for(uint32_t j=0;j<BUCKET_SIZE;++j) {
                hash_table[turn][i].slot[j].hash_key = i+1;
            }
        }
    }
}

inline hash_bucket* get_bucket(const Board& board) {
    return &hash_table[board.turn][board.hash_key % hash_size];
}

inline hash_slot* find_key(const Board& board, hash_bucket* bucket) {
    for(uint32_t i=0; i<BUCKET_SIZE; ++i) {
        if(bucket->slot[i].hash_key == board.hash_key) {
            for(int j=i;j>0;--j) {
                std::swap(bucket->slot[j-1], bucket->slot[j]);
            }
            return &bucket->slot[0];
        }
    }
    return 0;
}

/* search the given position in the hash */
inline hash_slot* hash_find(const Board& board) {
    hash_bucket* bucket = get_bucket(board);

    return find_key(board, bucket);
}

/* insert the given position in the hash */
inline void hash_insert(const Board& board, int depth, int lower_bound, int upper_bound, Move move) {
    hash_slot* cand = hash_find(board);

    if(cand == 0) {
        hash_bucket* bucket = get_bucket(board);
        for(int i=BUCKET_SIZE-1;i>0;--i) {
            bucket->slot[i]=bucket->slot[i-1];
        }
        cand = &bucket->slot[0];
    }

    if(cand->hash_key == board.hash_key) {
        if(cand->depth > depth) {
            return;
        } else if(cand->depth == depth) {
            lower_bound = std::max(lower_bound, cand->lower_bound);
            upper_bound = std::min(upper_bound, cand->upper_bound);
        }
    }

    cand->hash_key = board.hash_key;
    cand->lower_bound = lower_bound;
    cand->upper_bound = upper_bound;
    cand->depth = depth;
    cand->move = move;
}

/* initialize the hash table */
namespace {

    struct init_hash_table {
        init_hash_table() {
            hash_table[0]=hash_table[1]=0;
            hash_set_size(1000);
        }
    } _init_hash_table;
}

#endif
