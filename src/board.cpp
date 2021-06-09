#include "board.hh"

uint64_t hash_code[64][8][2];
uint64_t hash_code_turn;

void init_hash_code() {
  for (int p = 0; p < 64; ++p) {
    for (int owner = 0; owner < 2; ++owner) {
      for (int type = 0; type < 8; ++type) {
        hash_code[p][type][owner] = rand64();
      }
    }
  }
  hash_code_turn = rand64();
}

namespace {
struct __init_hash_code_t__ {
  __init_hash_code_t__() { init_hash_code(); }
} __init_hash_code__;
} // namespace
