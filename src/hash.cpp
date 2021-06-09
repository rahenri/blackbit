#include "hash.hh"

size_t hash_size = 0;

hash_bucket *hash_table[2] = {0, 0};

/* initialize the hash table */
namespace {

struct init_hash_table {
  init_hash_table() { hash_set_size(1000); }
} _init_hash_table;
} // namespace

