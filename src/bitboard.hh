#ifndef BITBOARD_HH
#define BITBOARD_HH

#include <stdint.h>
#include <cstdio>
#include <algorithm>

#include "pieces.hh"
#include "debug.hh"

uint8_t pop_count_table[1<<16];
uint8_t ctz_table[1<<16];

static const uint32_t col_mask = 0x01010101u;

static const uint32_t col_rotate_code = 0x10204080u;

static const uint32_t diag1_mask[][2] = {
    {0x00000001u, 0x00000000u},
    {0x00000102u, 0x00000000u},
    {0x00010204u, 0x00000000u},
    {0x01020408u, 0x00000000u},
    {0x02040810u, 0x00000001u},
    {0x04081020u, 0x00000102u},
    {0x08102040u, 0x00010204u},
    {0x10204080u, 0x01020408u},
    {0x20408000u, 0x02040810u},
    {0x40800000u, 0x04081020u},
    {0x80000000u, 0x08102040u},
    {0x00000000u, 0x10204080u},
    {0x00000000u, 0x20408000u},
    {0x00000000u, 0x40800000u},
    {0x00000000u, 0x80000000u}
};


const static uint32_t diag2_mask[][2] = {
    {0x00000080u, 0x00000000u},
    {0x00008040u, 0x00000000u},
    {0x00804020u, 0x00000000u},
    {0x80402010u, 0x00000000u},
    {0x40201008u, 0x00000080u},
    {0x20100804u, 0x00008040u},
    {0x10080402u, 0x00804020u},
    {0x08040201u, 0x80402010u},
    {0x04020100u, 0x40201008u},
    {0x02010000u, 0x20100804u},
    {0x01000000u, 0x10080402u},
    {0x00000000u, 0x08040201u},
    {0x00000000u, 0x04020100u},
    {0x00000000u, 0x02010000u},
    {0x00000000u, 0x01000000u}
};

static const uint32_t diag1_number[] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     1,  2,  3,  4,  5,  6,  7,  8,
     2,  3,  4,  5,  6,  7,  8,  9,
     3,  4,  5,  6,  7,  8,  9, 10,
     4,  5,  6,  7,  8,  9, 10, 11,
     5,  6,  7,  8,  9, 10, 11, 12,
     6,  7,  8,  9, 10, 11, 12, 13,
     7,  8,  9, 10, 11, 12, 13, 14
};

static const uint32_t diag2_number[] = {
     7,  6,  5,  4,  3,  2,  1,  0,
     8,  7,  6,  5,  4,  3,  2,  1,
     9,  8,  7,  6,  5,  4,  3,  2,
    10,  9,  8,  7,  6,  5,  4,  3,
    11, 10,  9,  8,  7,  6,  5,  4,
    12, 11, 10,  9,  8,  7,  6,  5,
    13, 12, 11, 10,  9,  8,  7,  6,
    14, 13, 12, 11, 10,  9,  8,  7
};

const static uint32_t diag_rotate_code = 0x01010101;

inline int pop_count8(uint32_t n) {
    return pop_count_table[n];
}

inline int pop_count16(uint32_t n) {
    return pop_count_table[n];
}

inline int pop_count32(uint32_t n) {
    return pop_count_table[n&0xffff] + pop_count_table[n>>16];
}

#define _min(a, b) ((a)<(b))?(a):(b)

class BitBoard {
    public:

        inline BitBoard() {}

        inline BitBoard(uint64_t value) : m_board64(value) { }

        inline BitBoard(uint32_t lower, uint32_t upper) : 
            m_lower(lower), m_upper(upper) { }

        inline BitBoard operator|(const BitBoard& bb) const {
            return BitBoard(m_lower|bb.m_lower, m_upper|bb.m_upper);
        }
        inline BitBoard& operator|=(const BitBoard& bb) {
            m_lower |= bb.m_lower;
            m_upper |= bb.m_upper;
            return *this;
        }

        inline BitBoard operator&(const BitBoard& bb) const {
            return BitBoard(m_lower&bb.m_lower, m_upper&bb.m_upper);
        }
        inline BitBoard& operator&=(const BitBoard& bb) {
            m_lower &= bb.m_lower;
            m_upper &= bb.m_upper;
            return *this;
        }

        inline BitBoard operator^(const BitBoard& bb) const {
            return BitBoard(m_lower^bb.m_lower, m_upper^bb.m_upper);
        }
        inline BitBoard& operator^=(const BitBoard& bb) {
            m_lower ^= bb.m_lower;
            m_upper ^= bb.m_upper;
            return *this;
        }

        inline BitBoard operator>>(int shift) const {
            if(shift<=0) {
                return *this;
            } else if(shift < 32) {
                return BitBoard((m_lower >> shift) | (m_upper << (32-shift)),
                        m_upper >> shift);
            } else if(shift < 64) {
                return BitBoard(m_upper >> (shift-32), 0);
            } else {
                return BitBoard(0);
            }
        }
        inline BitBoard& operator>>=(int shift) {
            if(shift <= 0) {
                /* do nothing */
            } else if(shift < 32) {
                m_lower = (m_lower >> shift) | (m_upper << (32-shift));
                m_upper = m_upper >> shift;
            } else if(shift<64) {
                m_lower = m_upper >> (shift-32);
                m_upper = 0;
            } else {
                m_lower = m_upper = 0;
            }
            return *this;
        }

        inline BitBoard operator<<(int shift) const {
            if(shift<32) {
                if(shift<=0) {
                    return *this;
                } else {
                    return BitBoard((m_lower << shift),
                            (m_upper << shift) | (m_lower >> (32-shift)));
                }
            } else {
                if(shift < 64) {
                    return BitBoard(0, m_lower << (shift-32));
                } else {
                    return BitBoard(0);
                }
            }
        }
        inline BitBoard& operator<<=(int shift) {
            if(shift < 32) {
                if(shift > 0) {
                    m_upper = (m_upper << shift) | (m_lower >> (32-shift));
                    m_lower = m_lower << shift;
                }
            } else {
                if(shift < 64) {
                    m_upper = m_lower << (shift-32);
                    m_lower = 0;
                } else {
                    m_lower = m_upper = 0;
                }
            }
            return *this;
        }

        inline BitBoard operator~() const {
            return BitBoard(~m_lower, ~m_upper);
        }

        inline bool operator==(const BitBoard& bb) {
            return m_lower == bb.m_lower and m_upper == bb.m_upper;
        }
        inline bool operator!=(const BitBoard& bb) {
            return m_lower != bb.m_lower or m_upper != bb.m_upper;
        }

        inline BitBoard& set(int place) {
            if(place < 32) {
                m_lower |= (1<<place);
            } else {
                m_upper |= (1<<(place-32));
            }
            return *this;
        }

        inline BitBoard& clear(int place) {
            if(place < 32) {
                m_lower &= ~(1<<place);
            } else {
                m_upper &= ~(1<<(place-32));
            }
            return *this;
        }

        inline BitBoard& invert(int place) {
            if(place < 32) {
                m_lower ^= (1<<place);
            } else {
                m_upper ^= (1<<(place-32));
            }
            return *this;
        }

        inline void clear() {
            m_upper = m_lower = 0;
        }

        bool inline empty() const {
            return m_upper == 0 and m_lower == 0;
        }

        inline bool is_set(int place) const {
            if(place < 32) {
                return m_lower & (1<<place);
            } else {
                return m_upper & (1<<(place-32));
            }
        }

        inline int pop_count() const {
            return pop_count32(m_lower) + pop_count32(m_upper);
        }

        inline int get_line(int line) const {
            return (m_board[line/4] >> ((line%4)*8)) & 0xff;
        }

        inline int get_col(int col) const {
            return ((((m_lower>>col) & col_mask) * col_rotate_code)
                    >> 28) |
                ((((m_upper>>col) & col_mask) * col_rotate_code)
                 >> 24);
        }

        inline int get_col_pop(int col) const {
            return pop_count8(get_col(col));
        }

        inline int get_diag1(int d) const {
            return (((m_lower & diag1_mask[d][0]) * diag_rotate_code)
                    >> 24) |
                (((m_upper & diag1_mask[d][1]) * diag_rotate_code)
                 >> 24);
        }

        inline int get_diag2(int d) const {
            return (((m_lower & diag2_mask[d][0]) * diag_rotate_code)
                    >> 24) |
                (((m_upper & diag2_mask[d][1]) * diag_rotate_code)
                 >> 24);
        }

        inline int get_one_place() const {
            if(m_lower) {
                if(m_lower&0xffff)
                    return ctz_table[m_lower&0xffff];
                else
                    return ctz_table[m_lower>>16]+16;
            } else {
                if(m_upper&0xffff)
                    return ctz_table[m_upper&0xffff]+32;
                else
                    return ctz_table[m_upper>>16]+48;
            }
            //return __builtin_ctzll(m_board64);
        }

        inline int pop_place() {
            int p = get_one_place();
            invert(p);
            return p;
        }

    private:
        union {
            struct {
                uint32_t m_lower, m_upper;
            };
            uint32_t m_board[2];
            uint64_t m_board64;
        };
};

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


/* not BB */
inline int get_lin(int place) {
    return place >> 3;
}

/* not BB */
inline int get_col(int place) {
    return place & 7;
}

/* not BB */
inline int make_place(int lin, int col) {
    return col | (lin << 3);
}

inline BitBoard get_pawn_noncapture_moves(int color, int place, BitBoard blockers){
    BitBoard resp = pawn_moves[color][place] & (~blockers);
    if(not resp.empty()) {
        resp |= pawn_moves2[color][place] & (~blockers);
    }
    return resp;
}

BitBoard get_pawn_capture_moves(int color, int place, BitBoard blockers) {
    return pawn_captures[color][place] & blockers;
}

BitBoard get_pawn_capture_promotion_moves(int color, int place, BitBoard blockers) {
    return (pawn_captures[color][place] & blockers)
        | (pawn_promotion[color][place] & (~blockers));
}

BitBoard get_pawn_moves(int color, int place, BitBoard blockers) {
    return get_pawn_noncapture_moves(color, place, blockers) |
           get_pawn_capture_moves(color, place, blockers);
}

BitBoard get_knight_moves(int place) {
    return knight_moves[place];
}

BitBoard get_bishop_moves(int place, BitBoard blockers) {
    int diag1 = diag1_number[place], diag2 = diag2_number[place];
    int diag1_code = blockers.get_diag1(diag1);
    int diag2_code = blockers.get_diag2(diag2);

    return bishop_diag1_moves[place][diag1_code]
         | bishop_diag2_moves[place][diag2_code];
}

BitBoard get_rook_moves(int place, BitBoard blockers) {
    int lin = get_lin(place), col = get_col(place);
    int lin_code = blockers.get_line(lin);
    int col_code = blockers.get_col(col);

    return (rook_lin_moves[col][lin_code] << (lin*8))
         | (rook_col_moves[lin][col_code] << col);
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
    return BitBoard(col_mask<<get_col(place),col_mask<<get_col(place));
}

void init_bitboard() {
    using namespace std;
    /* init_popcount table */
    pop_count_table[0] = 0;
    for(int i = 1; i < (1<<16); ++i) {
        pop_count_table[i] = pop_count_table[i & (i-1)] + 1;
    }

    /* init ctz table */ 
    for(int i=0;i<(1<<16);++i) {
        ctz_table[i] = __builtin_ctz(i);
    }


    /* init pawn tables */
    for(int o=0;o<2;++o) {
        for(int p=0;p<64;++p) {
            pawn_moves[o][p].clear();
            pawn_moves2[o][p].clear();
            pawn_captures[o][p].clear();
            pawn_promotion[o][p].clear();
            pawn_passed_mask[o][p].clear();
        }
    }

    for(int p = 8; p < 64 - 8; ++p) {
        int col = get_col(p);
        int lin = get_lin(p);
        /* movimento para frente */
        pawn_moves[BLACK][p].set(p-8);
        pawn_moves[WHITE][p].set(p+8);

        if(col > 0) {
            /* captura para esquerda */
            pawn_captures[BLACK][p].set(p-1-8);
            pawn_captures[WHITE][p].set(p-1+8);
        }

        if(col < 7) {
            /* captura para a direita */
            pawn_captures[BLACK][p].set(p+1-8);
            pawn_captures[WHITE][p].set(p+1+8);
        }

        if(lin == 1) {
            pawn_promotion[BLACK][p].set(p-8);
            pawn_moves2[WHITE][p].set(p+16);
        }

        if(lin == 6) {
            pawn_promotion[WHITE][p].set(p+8);
            pawn_moves2[BLACK][p].set(p-16);
        }

        /* set passed mask */
        for(int c1 = max(0, col - 1); c1 <= min(7, col+1); ++c1) {
            for(int l1 = lin+1;l1<8;++l1) {
                pawn_passed_mask[WHITE][p].set(make_place(l1, c1));
            }
            for(int l1 = lin-1;l1>=0;--l1) {
                pawn_passed_mask[BLACK][p].set(make_place(l1, c1));
            }
        }
    }

    /* knight tables */
    static const int ndl[] = { 2, 1,-1,-2,-2,-1, 1, 2};
    static const int ndc[] = { 1, 2, 2, 1,-1,-2,-2,-1};
    for(int lin = 0; lin < 8;++lin) {
        for(int col = 0; col < 8;++col) {
            int p = make_place(lin, col);
            knight_moves[p].clear();
            for(int i=0;i<8;++i) {
                int lin1 = lin + ndl[i], col1 = col + ndc[i];
                if(is_valid_place(lin1, col1)) {
                    knight_moves[p].set(make_place(lin1, col1));
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
            king_moves[p].clear();
            for(int i=0;i<8;++i) {
                int lin1 = lin + kdl[i], col1 = col + kdc[i];
                if(is_valid_place(lin1, col1)) {
                    king_moves[p].set(make_place(lin1, col1));
                }
            }
        }
    }

    /* rook tables */
    static const int rd[] = {1, -1};
    int tmp[8];
    for(int i = 0; i < 8; ++i) {
        for(int m = 0; m < (1<<8); ++m) {
            /* unpack mask */
            for(int j = 0; j < 8; ++j) {
                tmp[j] = (m >> j) & 1;
            }
            BitBoard bb1(0), bb2(0);
            for(int j=0;j<2;++j) {
                int c = 0;
                for(c = i+rd[j];c<8 and c>=0 and tmp[c]==0;c+=rd[j]) {
                    bb1.set(make_place(0, c));
                    bb2.set(make_place(c, 0));
                }
                if(c < 8 and c >= 0) {
                    bb1.set(make_place(0, c));
                    bb2.set(make_place(c, 0));
                }
            }
            rook_lin_moves[i][m] = bb1;
            rook_col_moves[i][m] = bb2;
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
                /* unpack mask */
                for(int i=0;i<8;++i) {
                    tmp[i] = (m >> i) & 1;
                }

                int c = 0, l = 0;
                BitBoard bb(0);

                for(int i=0;i<2;++i) {
                    for(c=col+bdc1[i], l=lin+bdl1[i]; is_valid_place(l, c) and tmp[c]==0; c+=bdc1[i], l+=bdl1[i]) {
                        bb.set(make_place(l, c));
                    }
                    if(is_valid_place(l, c)) {
                        bb.set(make_place(l, c));
                    }
                }
                bishop_diag1_moves[p][m] = bb;

                bb = 0;
                for(int i=0;i<2;++i) {
                    for(c=col+bdc2[i], l=lin+bdl2[i]; is_valid_place(l, c) and tmp[c]==0; c+=bdc2[i], l+=bdl2[i]) {
                        bb.set(make_place(l, c));
                    }
                    if(is_valid_place(l, c)) {
                        bb.set(make_place(l, c));
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
                    bb.set(make_place(l, col-1));
                if(col < 7)
                    bb.set(make_place(l, col+1));
            }
            neighbor_col[p] = bb;
        }
    }

}

void print_bitboard(FILE* f, BitBoard b) {
    char tmp [8][16];
    for(int l=0;l<8;++l) {
        for(int c=0;c<8;++c) {
            tmp[l][c] = '0' + b.is_set(make_place(l, c));
        }
        tmp[l][8] = 0;
    }
    for(int l=7;l>=0;--l) {
        fprintf(f, "%s\n", tmp[l]);
    }
    fprintf(f, "\n");
}

namespace {
    struct __init_bitboard_t__ {
        __init_bitboard_t__() {
            init_bitboard();
        }
    } __init_bitboard__;
}

#endif
