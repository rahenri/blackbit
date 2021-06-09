#include "bitboard.h"

uint8_t pop_count_table[1 << 16];
uint8_t ctz_table[1 << 16];

BitBoard pawn_moves[2][64];
BitBoard pawn_moves2[2][64];
BitBoard pawn_captures[2][64];
BitBoard pawn_promotion[2][64];
BitBoard pawn_passed_mask[2][64];

BitBoard neighbor_col[64];

BitBoard rook_lin_moves[8][256];
BitBoard rook_col_moves[8][256];

BitBoard bishop_diag1_moves[64][256];
BitBoard bishop_diag2_moves[64][256];

BitBoard knight_moves[64];

BitBoard king_moves[64];

void init_bitboard() {
  using namespace std;
  /* init_popcount table */
  pop_count_table[0] = 0;
  for (int i = 1; i < (1 << 16); ++i) {
    pop_count_table[i] = pop_count_table[i & (i - 1)] + 1;
  }

  /* init ctz table */
  for (int i = 0; i < (1 << 16); ++i) {
    ctz_table[i] = __builtin_ctz(i);
  }

  /* init pawn tables */
  for (int o = 0; o < 2; ++o) {
    for (int p = 0; p < 64; ++p) {
      pawn_moves[o][p].clear();
      pawn_moves2[o][p].clear();
      pawn_captures[o][p].clear();
      pawn_promotion[o][p].clear();
      pawn_passed_mask[o][p].clear();
    }
  }

  for (int p = 8; p < 64 - 8; ++p) {
    int col = get_col(p);
    int lin = get_lin(p);
    /* movimento para frente */
    pawn_moves[BLACK][p].set(p - 8);
    pawn_moves[WHITE][p].set(p + 8);

    if (col > 0) {
      /* captura para esquerda */
      pawn_captures[BLACK][p].set(p - 1 - 8);
      pawn_captures[WHITE][p].set(p - 1 + 8);
    }

    if (col < 7) {
      /* captura para a direita */
      pawn_captures[BLACK][p].set(p + 1 - 8);
      pawn_captures[WHITE][p].set(p + 1 + 8);
    }

    if (lin == 1) {
      pawn_promotion[BLACK][p].set(p - 8);
      pawn_moves2[WHITE][p].set(p + 16);
    }

    if (lin == 6) {
      pawn_promotion[WHITE][p].set(p + 8);
      pawn_moves2[BLACK][p].set(p - 16);
    }

    /* set passed mask */
    for (int c1 = max(0, col - 1); c1 <= min(7, col + 1); ++c1) {
      for (int l1 = lin + 1; l1 < 8; ++l1) {
        pawn_passed_mask[WHITE][p].set(make_place(l1, c1));
      }
      for (int l1 = lin - 1; l1 >= 0; --l1) {
        pawn_passed_mask[BLACK][p].set(make_place(l1, c1));
      }
    }
  }

  /* knight tables */
  static const int ndl[] = {2, 1, -1, -2, -2, -1, 1, 2};
  static const int ndc[] = {1, 2, 2, 1, -1, -2, -2, -1};
  for (int lin = 0; lin < 8; ++lin) {
    for (int col = 0; col < 8; ++col) {
      int p = make_place(lin, col);
      knight_moves[p].clear();
      for (int i = 0; i < 8; ++i) {
        int lin1 = lin + ndl[i], col1 = col + ndc[i];
        if (is_valid_place(lin1, col1)) {
          knight_moves[p].set(make_place(lin1, col1));
        }
      }
    }
  }

  /* king tables */
  static const int kdl[] = {1, 1, 1, 0, -1, -1, -1, 0};
  static const int kdc[] = {1, 0, -1, -1, -1, 0, 1, 1};
  for (int lin = 0; lin < 8; ++lin) {
    for (int col = 0; col < 8; ++col) {
      int p = make_place(lin, col);
      king_moves[p].clear();
      for (int i = 0; i < 8; ++i) {
        int lin1 = lin + kdl[i], col1 = col + kdc[i];
        if (is_valid_place(lin1, col1)) {
          king_moves[p].set(make_place(lin1, col1));
        }
      }
    }
  }

  /* rook tables */
  static const int rd[] = {1, -1};
  int tmp[8];
  for (int i = 0; i < 8; ++i) {
    for (int m = 0; m < (1 << 8); ++m) {
      /* unpack mask */
      for (int j = 0; j < 8; ++j) {
        tmp[j] = (m >> j) & 1;
      }
      BitBoard bb1(0), bb2(0);
      for (int j = 0; j < 2; ++j) {
        int c = 0;
        for (c = i + rd[j]; c < 8 and c >= 0 and tmp[c] == 0; c += rd[j]) {
          bb1.set(make_place(0, c));
          bb2.set(make_place(c, 0));
        }
        if (c < 8 and c >= 0) {
          bb1.set(make_place(0, c));
          bb2.set(make_place(c, 0));
        }
      }
      rook_lin_moves[i][m] = bb1;
      rook_col_moves[i][m] = bb2;
    }
  }

  /* bishop tables */
  static const int bdl1[] = {1, -1};
  static const int bdc1[] = {-1, 1};
  static const int bdl2[] = {1, -1};
  static const int bdc2[] = {1, -1};
  for (int lin = 0; lin < 8; ++lin) {
    for (int col = 0; col < 8; ++col) {
      int p = make_place(lin, col);
      for (int m = 0; m < 256; ++m) {
        /* unpack mask */
        for (int i = 0; i < 8; ++i) {
          tmp[i] = (m >> i) & 1;
        }

        int c = 0, l = 0;
        BitBoard bb(0);

        for (int i = 0; i < 2; ++i) {
          for (c = col + bdc1[i], l = lin + bdl1[i];
               is_valid_place(l, c) and tmp[c] == 0;
               c += bdc1[i], l += bdl1[i]) {
            bb.set(make_place(l, c));
          }
          if (is_valid_place(l, c)) {
            bb.set(make_place(l, c));
          }
        }
        bishop_diag1_moves[p][m] = bb;

        bb = 0;
        for (int i = 0; i < 2; ++i) {
          for (c = col + bdc2[i], l = lin + bdl2[i];
               is_valid_place(l, c) and tmp[c] == 0;
               c += bdc2[i], l += bdl2[i]) {
            bb.set(make_place(l, c));
          }
          if (is_valid_place(l, c)) {
            bb.set(make_place(l, c));
          }
        }
        bishop_diag2_moves[p][m] = bb;
      }
    }
  }

  /* init neighbor col table */
  for (int lin = 0; lin < 8; ++lin) {
    for (int col = 0; col < 8; ++col) {
      int p = make_place(lin, col);
      BitBoard bb = 0;
      for (int l = 0; l < 8; ++l) {
        if (col > 0)
          bb.set(make_place(l, col - 1));
        if (col < 7)
          bb.set(make_place(l, col + 1));
      }
      neighbor_col[p] = bb;
    }
  }
}

namespace {
struct __init_bitboard_t__ {
  __init_bitboard_t__() { init_bitboard(); }
} __init_bitboard__;
} // namespace
