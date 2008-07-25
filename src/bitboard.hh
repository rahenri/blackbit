#ifndef BITBOARD_HH
#define BITBOARD_HH

#include <stdint.h>
#include <cstdio>
#include <algorithm>

#include "pieces.hh"
#include "debug.hh"

typedef uint64_t BitBoard;

BitBoard pawn_moves[2][64];
BitBoard pawn_moves2[2][64];
BitBoard pawn_captures[2][64];
BitBoard pawn_promotion[2][64];
BitBoard pawn_passed_mask[2][64];

BitBoard neighbor_col[64];

BitBoard rook_lin_moves[64][256];
BitBoard rook_col_moves[64][256];

BitBoard bishop_diag1_moves[64][256];
BitBoard bishop_diag2_moves[64][256];

BitBoard knight_moves[64];

BitBoard king_moves[64];

uint8_t popcount_table[0x10000];

const BitBoard lin_mask[] = {
    0xff00000000000000ull,
    0x00ff000000000000ull,
    0x0000ff0000000000ull,
    0x000000ff00000000ull,
    0x00000000ff000000ull,
    0x0000000000ff0000ull,
    0x000000000000ff00ull,
    0x00000000000000ffull,
};

const BitBoard col_mask[] = {
    0x0101010101010101ull,
    0x0202020202020202ull,
    0x0404040404040404ull,
    0x0808080808080808ull,
    0x1010101010101010ull,
    0x2020202020202020ull,
    0x4040404040404040ull,
    0x8080808080808080ull,
};

const BitBoard diag1_mask[] = {
    0x0000000000000001ull,
    0x0000000000000102ull,
    0x0000000000010204ull,
    0x0000000001020408ull,
    0x0000000102040810ull,
    0x0000010204081020ull,
    0x0001020408102040ull,
    0x0102040810204080ull,
    0x0204081020408000ull,
    0x0408102040800000ull,
    0x0810204080000000ull,
    0x1020408000000000ull,
    0x2040800000000000ull,
    0x4080000000000000ull,
    0x8000000000000000ull,
};

const BitBoard diag2_mask[] = {
    0x0000000000000080ull,
    0x0000000000008040ull,
    0x0000000000804020ull,
    0x0000000080402010ull,
    0x0000008040201008ull,
    0x0000804020100804ull,
    0x0080402010080402ull,
    0x8040201008040201ull,
    0x4020100804020100ull,
    0x2010080402010000ull,
    0x1008040201000000ull,
    0x0804020100000000ull,
    0x0402010000000000ull,
    0x0201000000000000ull,
    0x0100000000000000ull,
};

const BitBoard col_rotate_code[] = {
    0x0102040810204080ull,
    0x0081020408102040ull,
    0x0040810204081020ull,
    0x0020408102040810ull,
    0x0010204081020408ull,
    0x0008102040810204ull,
    0x0004081020408102ull,
    0x0002040810204081ull,
};

const int col_rotate_rshift = 56;

const uint32_t diag1_number[] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     1,  2,  3,  4,  5,  6,  7,  8,
     2,  3,  4,  5,  6,  7,  8,  9,
     3,  4,  5,  6,  7,  8,  9, 10,
     4,  5,  6,  7,  8,  9, 10, 11,
     5,  6,  7,  8,  9, 10, 11, 12,
     6,  7,  8,  9, 10, 11, 12, 13,
     7,  8,  9, 10, 11, 12, 13, 14
};

const uint32_t diag2_number[] = {
     7,  6,  5,  4,  3,  2,  1,  0,
     8,  7,  6,  5,  4,  3,  2,  1,
     9,  8,  7,  6,  5,  4,  3,  2,
    10,  9,  8,  7,  6,  5,  4,  3,
    11, 10,  9,  8,  7,  6,  5,  4,
    12, 11, 10,  9,  8,  7,  6,  5,
    13, 12, 11, 10,  9,  8,  7,  6,
    14, 13, 12, 11, 10,  9,  8,  7
};

const BitBoard diag1_rotate_code[] = {
    0x0100000000000000ull,
    0x0101000000000000ull,
    0x0101010000000000ull,
    0x0101010100000000ull,
    0x0101010101000000ull,
    0x0101010101010000ull,
    0x0101010101010100ull,
    0x0101010101010101ull,
    0x0001010101010101ull,
    0x0000010101010101ull,
    0x0000000101010101ull,
    0x0000000001010101ull,
    0x0000000000010101ull,
    0x0000000000000101ull,
    0x0000000000000001ull
};

const BitBoard diag2_rotate_code[] = {
    0x0100000000000000ull,
    0x0101000000000000ull,
    0x0101010000000000ull,
    0x0101010100000000ull,
    0x0101010101000000ull,
    0x0101010101010000ull,
    0x0101010101010100ull,
    0x0101010101010101ull,
    0x0001010101010101ull,
    0x0000010101010101ull,
    0x0000000101010101ull,
    0x0000000001010101ull,
    0x0000000000010101ull,
    0x0000000000000101ull,
    0x0000000000000001ull
};

const int diag1_rotate_rshift = 56;

const int diag2_rotate_rshift = 56;


int popcount(BitBoard b) {
    int r = 0;
    for(int i=0;i<4;++i) {
        r += popcount_table[b&0xffff];
        b >>= 16;
    }
    return r;
}


inline int get_lin(int place) {
    return place >> 3;
}

inline int get_col(int place) {
    return place & 7;
}

inline BitBoard get_bb_by_place(int place) {
    return (1ull<<place);
}

inline int make_place(int lin, int col) {
    return col | (lin << 3);
}

int get_lin_code(int lin, BitBoard b) {
    return (b >> (lin*8)) & 0xff;
}

int get_col_code(int col, BitBoard b) {
    return ((b & col_mask[col]) * col_rotate_code[col]) >> col_rotate_rshift;
}

int get_col_count(int place, BitBoard b) {
    int col = get_col(place);
    return popcount_table[get_col_code(col, b)];
}

int get_diag1(int p) {
    return diag1_number[p];
}

int get_diag2(int p) {
    return diag2_number[p];
}

int get_diag1_code(int d, BitBoard b) {
    return ((b & diag1_mask[d]) * diag1_rotate_code[d]) >> diag1_rotate_rshift;
}

int get_diag2_code(int d, BitBoard b) {
    return ((b & diag2_mask[d]) * diag2_rotate_code[d]) >> diag2_rotate_rshift;
}

void bb_clear_place(BitBoard& b, int place) {
    b &= ~(get_bb_by_place(place));
}

void bb_invert_place(BitBoard& b, int place) {
    b ^= get_bb_by_place(place);
}

void bb_set_place(BitBoard& b, int place) {
    b |= get_bb_by_place(place);
}

bool bb_is_place_set(BitBoard b, int place) {
    return (b >> place) & 1;
}

int bb_get_one_place(const BitBoard& b) {
    return __builtin_ctzll(b);
}

int bb_pop_place(BitBoard& b) {
    int resp = bb_get_one_place(b);
    bb_invert_place(b, resp);
    return resp;
}

int bb_count_places(const BitBoard& b) {
    //return __builtin_popcountll(b);
    return popcount(b);
}

BitBoard get_pawn_noncapture_moves(int color, int place, BitBoard blockers) {
    BitBoard resp = pawn_moves[color][place] & (~blockers);
    if(resp) {
        resp |= pawn_moves2[color][place] & (~blockers);
    }
    return resp;
}

BitBoard get_pawn_capture_moves(int color, int place, BitBoard blockers) {
    return pawn_captures[color][place] & blockers;
}

BitBoard get_pawn_capture_promotion_moves(int color, int place, BitBoard blockers) {
    return (pawn_captures[color][place] & blockers) | (pawn_promotion[color][place] & (~blockers));
}

BitBoard get_pawn_moves(int color, int place, BitBoard blockers) {
    return get_pawn_noncapture_moves(color, place, blockers) |
           get_pawn_capture_moves(color, place, blockers);
}

BitBoard get_knight_moves(int place) {
    return knight_moves[place];
}

BitBoard get_bishop_moves(int place, BitBoard blockers) {
    int diag1 = get_diag1(place), diag2 = get_diag2(place);
    int diag1_code = get_diag1_code(diag1, blockers);
    int diag2_code = get_diag2_code(diag2, blockers);

    return bishop_diag1_moves[place][diag1_code]
         | bishop_diag2_moves[place][diag2_code];
}

BitBoard get_rook_moves(int place, BitBoard blockers) {
    int lin = get_lin(place), col = get_col(place);
    int lin_code = get_lin_code(lin, blockers);
    int col_code = get_col_code(col, blockers);

    return rook_lin_moves[place][lin_code]
         | rook_col_moves[place][col_code];
}


BitBoard get_queen_moves(int place, BitBoard blockers) {
    return get_rook_moves(place, blockers)
         | get_bishop_moves(place, blockers);
}


BitBoard get_king_moves(int place) {
    return king_moves[place];
}

bool is_valid_place(int lin, int col) {
    return lin>=0 and lin<8 and col>=0 and col<8;
}

BitBoard get_passed_pawn_mask(int color, int place) {
    return pawn_passed_mask[color][place];
}

BitBoard get_neighbor_col_mask(int place) {
    return neighbor_col[place];
}

BitBoard get_col_mask(int place) {
    return col_mask[get_col(place)];
}

void init_bitboard() {
    using namespace std;
    /* init_popcount table */
    popcount_table[0] = 0;
    for(int i = 1; i < 0x10000; ++i) {
        popcount_table[i] = popcount_table[i & (i-1)] + 1;
    }

    /* init pawn tables */
    for(int o=0;o<2;++o) {
        for(int p=0;p<64;++p) {
            pawn_moves[o][p] = 0;
            pawn_moves2[o][p] = 0;
            pawn_captures[o][p] = 0;
            pawn_promotion[o][p] = 0;
            pawn_passed_mask[o][p] = 0;
        }
    }

    for(int p = 8; p < 64 - 8; ++p) {
        int col = get_col(p);
        int lin = get_lin(p);
        /* movimento para frente */
        bb_set_place(pawn_moves[BLACK][p], p-8);
        bb_set_place(pawn_moves[WHITE][p], p+8);

        if(col > 0) {
            /* captura para esquerda */
            bb_set_place(pawn_captures[BLACK][p], p-1-8);
            bb_set_place(pawn_captures[WHITE][p], p-1+8);
        }

        if(col < 7) {
            /* captura para a direita */
            bb_set_place(pawn_captures[BLACK][p], p+1-8);
            bb_set_place(pawn_captures[WHITE][p], p+1+8);
        }

        if(lin == 1) {
            bb_set_place(pawn_promotion[BLACK][p], p-8);
            bb_set_place(pawn_moves2[WHITE][p], p+16);
        }

        if(lin == 6) {
            bb_set_place(pawn_promotion[WHITE][p], p+8);
            bb_set_place(pawn_moves2[BLACK][p], p-16);
        }

        /* set passed mask */
        for(int c1 = max(0, col - 1); c1 <= min(7, col+1); ++c1) {
            for(int l1 = lin+1;l1<8;++l1) {
                bb_set_place(pawn_passed_mask[WHITE][p], make_place(l1, c1));
            }
            for(int l1 = lin-1;l1>=0;--l1) {
                bb_set_place(pawn_passed_mask[BLACK][p], make_place(l1, c1));
            }
        }
    }

    /* knight tables */
    static const int ndl[] = { 2, 1,-1,-2,-2,-1, 1, 2};
    static const int ndc[] = { 1, 2, 2, 1,-1,-2,-2,-1};
    for(int lin = 0; lin < 8;++lin) {
        for(int col = 0; col < 8;++col) {
            int p = make_place(lin, col);
            knight_moves[p] = 0;
            for(int i=0;i<8;++i) {
                int lin1 = lin + ndl[i], col1 = col + ndc[i];
                if(is_valid_place(lin1, col1)) {
                    bb_set_place(knight_moves[p], make_place(lin1, col1));
                }
            }
        }
    }

    /* king tables */
    static const int kdl[] = { 1, 1, 1, 0,-1,-1,-1, 0};
    static const int kdc[] = { 1, 0,-1,-1,-1, 0, 1, 1};
    for(int lin = 0; lin < 8;++lin) {
        for(int col = 0; col < 8;++col) {
            int p = make_place(lin, col);
            king_moves[p] = 0;
            for(int i=0;i<8;++i) {
                int lin1 = lin + kdl[i], col1 = col + kdc[i];
                if(is_valid_place(lin1, col1)) {
                    bb_set_place(king_moves[p], make_place(lin1, col1));
                }
            }
        }
    }

    /* rook tables */
    static const int rd[] = {1, -1};
    int tmp[8];
    for(int lin = 0; lin < 8; ++lin) {
        for(int col = 0; col < 8;++col) {
            int p = make_place(lin, col);
            for(int m = 0; m < 256; ++m) {
                for(int i=0;i<8;++i) {
                    tmp[i] = (m >> i) & 1;
                }

                int c = 0, l = 0;
                BitBoard bb = 0;
                for(int i=0;i<2;++i) {
                    for(c = col+rd[i];c<8 and c>=0 and tmp[c]==0;c+=rd[i]) {
                        bb_set_place(bb, make_place(lin, c));
                    }
                    if(c < 8 and c >= 0) {
                        bb_set_place(bb, make_place(lin, c));
                    }
                }
                rook_lin_moves[p][m] = bb;

                bb = 0;
                for(int i=0;i<2;++i) {
                    for(l = lin+rd[i];l<8 and l>=0 and tmp[l]==0;l+=rd[i]) {
                        bb_set_place(bb, make_place(l, col));
                    }
                    if(l < 8 and l >= 0) {
                        bb_set_place(bb, make_place(l, col));
                    }
                }
                rook_col_moves[p][m] = bb;
            }
        }
    }

    /* bishop tables */
    static const int bdl1[] = { 1, -1};
    static const int bdc1[] = {-1,  1};
    static const int bdl2[] = { 1, -1};
    static const int bdc2[] = { 1, -1};
    for(int lin = 0; lin < 8; ++lin) {
        for(int col = 0; col < 8;++col) {
            int p = make_place(lin, col);
            for(int m = 0; m < 256; ++m) {
                for(int i=0;i<8;++i) {
                    tmp[i] = (m >> i) & 1;
                }

                int c = 0, l = 0;
                BitBoard bb = 0;

                for(int i=0;i<2;++i) {
                    for(c=col+bdc1[i], l=lin+bdl1[i]; is_valid_place(l, c) and tmp[c]==0; c+=bdc1[i], l+=bdl1[i]) {
                        bb_set_place(bb, make_place(l, c));
                    }
                    if(is_valid_place(l, c)) {
                        bb_set_place(bb, make_place(l, c));
                    }
                }
                bishop_diag1_moves[p][m] = bb;

                bb = 0;
                for(int i=0;i<2;++i) {
                    for(c=col+bdc2[i], l=lin+bdl2[i]; is_valid_place(l, c) and tmp[c]==0; c+=bdc2[i], l+=bdl2[i]) {
                        bb_set_place(bb, make_place(l, c));
                    }
                    if(is_valid_place(l, c)) {
                        bb_set_place(bb, make_place(l, c));
                    }
                }
                bishop_diag2_moves[p][m] = bb;
            }
        }
    }
    
    /* init neighbor col table */
    for(int lin = 0; lin<8;++lin) {
        for(int col=0;col<8;++col) {
            int p = make_place(lin, col);
            BitBoard bb = 0;
            for(int l=0;l<8;++l) {
                if(col > 0)
                    bb_set_place(bb, make_place(l, col-1));
                if(col < 7)
                    bb_set_place(bb, make_place(l, col+1));
            }
            neighbor_col[p] = bb;
        }
    }

}

void print_bitboard(FILE* f, BitBoard b) {
    char tmp [8][16];
    for(int l=0;l<8;++l) {
        for(int c=0;c<8;++c) {
            tmp[l][c] = '0' + (b % 2);
            b /= 2;
        }
        tmp[l][8] = 0;
    }
    for(int l=7;l>=0;--l) {
        fprintf(f, "%s\n", tmp[l]);
    }
    fprintf(f, "\n");
}

#endif
