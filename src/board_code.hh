#ifndef BOARD_CODE_HH
#define BOARD_CODE_HH

const int encode_table[][7] = {{ 0, 1, 2, 3, 4, 5, 6},
                               { 0, 7, 8, 9,10,11,12},
                               { 0, 0, 0, 0, 0, 0, 0}};

struct board_code {
    uint32_t code[8];

    void board_code(const Board& board) {
        this->encode(board);
    }

    void board_code() {
    }

    void encode(const Board& board) {
        for(int l=0;l<8;++l) {
            code[l] = 0;
            for(int c=0;c<8;++c) {
                code[l] = (code[l]<<4) | encode_table[board.b[l][c].owner][board.b[l][c].type];
            }
        }
    }

    bool operator==(const board_code& bc) const {
        for(int l=0;l<8;++l) {
            if(code[l] != bc.code[l])
                return false;
        }
        return true;
    }
    bool operator!=(const board_code& bc) const {
        return not this->operator==(bc);
    }

};

#endif
