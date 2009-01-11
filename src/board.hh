#ifndef BOARD_HH
#define BOARD_HH

#include "move.hh"
#include "debug.hh"
#include "random.hh"
#include "pieces.hh"
#include "bitboard.hh"

#include <string>

const int material_table[3][7]={{   0,-100,-300,-300,-500,-900,  0 },
                                {   0, 100, 300, 300, 500, 900,  0 },
                                {   0,   0,   0,   0,   0,   0,  0 }};

const int doubled_pawn_score[] = {20, -20};

const int isolated_pawn_score[2] = { 55,-55};

const int passed_pawn_score[2][8] = {{ 0,-50,-55,-61,-68,-76, 0},
                                     { 0, 50, 55, 61, 68, 76, 0}};


const int mobility_score[2][7] = {{ 0, 0,-4,-3,-2,-0, 0},
                                  { 0, 0, 4, 3, 2, 0, 0}};


struct Peca
{
    int16_t place;
    int8_t type;
    int8_t owner;
};

struct Coord {
    int16_t place;
};

struct Pos
{
    Pos() { }

    int8_t id;
    int8_t type;
    int8_t owner;
    int8_t __fill__;
};

struct MoveInfo {
    MoveInfo() { }

    int8_t castle;
    bool fez_passan;
    bool capturou;
    bool promoveu;
    int8_t passan_place;
    Pos p;
};

uint64_t hash_code[64][8][2];
uint64_t hash_code_turn;

void init_hash_code() {
    for(int p=0; p<64; ++p) {
        for(int owner=0;owner<2;++owner) {
            for(int type=0;type<8;++type) {
                hash_code[p][type][owner] = rand64();
            }
        }
    }
    hash_code_turn = rand64();
}

struct Board
{
    Pos b[64];
    Coord peca_table[2][8][16];
    int size_table[2][8];
    BitBoard bb_blockers[2];
    BitBoard bbPeca[2][8];
    int turn;
    int passan_place;
    int score;
    uint64_t hash_key;

    int move_count;
    uint64_t history[512];

    std::string result;

    Board() {
        clear();
    }

    void clear()
    {
        memset(b, 0, sizeof(b));
        passan_place = 64;
        memset(size_table, 0, sizeof(size_table));
        turn = WHITE;
        score = 0;
        hash_key = 0;
        for(int p=0;p<64;++p) {
            b[p].owner = NONE;
            b[p].type = CLEAR;
        }
        for(int i=0;i<2;++i) {
            for(int j=0;j<8;++j) {
                bbPeca[i][j] = 0;
            }
        }
        move_count = 0;
        bb_blockers[BLACK] = bb_blockers[WHITE] = 0;
    }

    void setFen(const std::string& fen)
    {
        int linha=7, coluna=0;
        int owner = 0, type = 0;

        this->clear();

        for(int i=0;fen[i]!=' ';++i) {
            if(fen[i]=='/') {
                --linha, coluna=0;
            } else if(isdigit(fen[i])) {
                coluna += fen[i]-'0';
            } else {
                owner = isupper(fen[i])?WHITE:BLACK;
                switch(tolower(fen[i])) {
                    case 'r': type = ROOK; break;
                    case 'n': type = KNIGHT; break;
                    case 'b': type = BISHOP; break;
                    case 'q': type = QUEEN; break;
                    case 'k': type = KING; break;
                    case 'p': type = PAWN; break;
                    default: type = 0;
                }
                insertPeca(make_place(linha, coluna), type, owner);
                ++coluna;
            }
        }
        for(int i=0;;++i) {
            if(fen[i]==' ') {
                for(;fen[i]==' ';++i);
                turn = (tolower(fen[i])=='w')?WHITE:BLACK;
                break;
            }
        }
    }


    void setInitial()
    {
        this->setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");
    }

    void erasePeca2(int place) {
        Pos p = b[place];
        int tmp = size_table[p.owner][p.type]--;
        if(p.id != tmp - 1) {
            peca_table[p.owner][p.type][p.id] = peca_table[p.owner][p.type][tmp-1];
            b[peca_table[p.owner][p.type][p.id].place].id = p.id;
        }
    }

    void erasePeca(int place) {
        int owner = b[place].owner;
        int type = b[place].type;

        /* update material count */
        score -= material_table[owner][type];

        /* update hahs code */
        hash_key ^= hash_code[place][type][owner];

        /* erase from piece list */
        erasePeca2(place);

        /* erase from matrix */
        b[place].type = CLEAR;
        b[place].owner = NONE;

        /* erase from bit board */
        bb_blockers[owner].invert(place);
        bbPeca[owner][type].invert(place);
    }

    void insertPeca2(int place, int type, int owner) {
        int tmp = size_table[owner][type]++;
        peca_table[owner][type][tmp].place = place;
        b[place].id = tmp;
    }

    void insertPeca(int place, int type, int owner) {
        /* update material count */
        score += material_table[owner][type];

        /* update hash code */
        hash_key ^= hash_code[place][type][owner];

        /* insrt to piece list */
        insertPeca2(place, type, owner);

        /* insrt to matrix */
        b[place].type = type;
        b[place].owner = owner;

        /* insert to bitboard */
        bb_blockers[owner].invert(place);
        bbPeca[owner][type].invert(place);
    }

    void movePeca(const Move& m) {
        Pos s;
        int owner = b[m.o].owner;
        int type = b[m.o].type;
        int id = b[m.o].id;

        /* update hash code */
        hash_key ^= hash_code[m.o][type][owner];
        hash_key ^= hash_code[m.d][type][owner];

        /* update piece list */
        peca_table[owner][type][id].place = m.d;

        /* update matrix */
        b[m.d] = b[m.o];
        b[m.o].type = CLEAR;
        b[m.o].owner = NONE;

        /* update bitboard */
        bb_blockers[owner].invert(m.o);
        bbPeca[owner][type].invert(m.o);
        bb_blockers[owner].invert(m.d);
        bbPeca[owner][type].invert(m.d);
    }

    void setType(int place, int type) {
        /* update material */
        score += material_table[b[place].owner][type] - material_table[b[place].owner][b[place].type];

        /* update hash code */
        hash_key ^= hash_code[place][type][b[place].owner];
        hash_key ^= hash_code[place][b[place].type][b[place].owner];

        /* update list */
        erasePeca2(place);
        insertPeca2(place, type, b[place].owner);

        /* update the bitboard */
        bbPeca[b[place].owner][b[place].type].invert(place);
        bbPeca[b[place].owner][type].invert(place);

        /* update matrix */
        b[place].type = type;
    }

    MoveInfo move(Move m) {
        MoveInfo mi;

        ASSERT(checkBoard());

        /* update the history */
        history[move_count ++] = hash_key;

        /* get the type of the piece to be moved */
        int type = b[m.o].type;

        /* copia peca capturada */
        if(b[m.d].type != CLEAR) {
            mi.p = b[m.d];
            mi.capturou = true;
            erasePeca(m.d);
        } else {
            mi.capturou = false;
        }

        /* movimenta a peca */
        movePeca(m);

        /* promove */
        if(type == PAWN and (m.d>=(64-8) or m.d<8)) {
            setType(m.d, QUEEN);
            mi.promoveu = true;
        } else {
            mi.promoveu = false;
        }

        /* marca posição de en passan */
        mi.passan_place = passan_place;
        if(type == PAWN and (m.d-m.o==16 or m.d-m.o==-16)) {
            passan_place = (m.o+m.d)/2;
        } else {
            passan_place = 64;
        }

        ASSERT(checkBoard());

        /* faz en passan */
        if(type == PAWN and (not mi.capturou) and m.dc() != m.oc()) {
            int p = make_place(m.ol(),m.dc());
            mi.p = b[p];
            mi.fez_passan = true;
            erasePeca(p);
        } else {
            mi.fez_passan = false;
        }


        /* faz roque */
        mi.castle = 0;
        if(type == KING) {
            if((m.d-m.o)==2) {
                movePeca(Move(m.d+1,m.d-1));
                mi.castle = 1;
            } else if((m.d-m.o)==-2) {
                movePeca(Move(m.d-2,m.d+1));
                mi.castle = 1;
            }
        }

        /* troca a vez */
        turn = OPPONENT(turn);
        hash_key ^= hash_code_turn;

        ASSERT(checkBoard());

        return mi;
    }

    void volta(Move m, const MoveInfo& mi) {

        ASSERT(checkBoard());

        /* troca a vez */
        hash_key ^= hash_code_turn;
        turn = OPPONENT(turn);

        /* desfaz roque */
        if(mi.castle) {
            if((m.d-m.o)==2) {
                movePeca(Move(m.d-1,m.d+1));
            } else {
                movePeca(Move(m.d+1,m.d-2));
            }
        }

        /* desfaz promocao */
        if(mi.promoveu) {
            setType(m.d, PAWN);
        }

        /* desfaz passan */
        if(mi.fez_passan) {
            insertPeca(make_place(m.ol(), m.dc()), mi.p.type, mi.p.owner);
        }

        /* volta a posição de en passan */
        passan_place = mi.passan_place;

        /* volta a peca */
        movePeca(Move(m.d, m.o));

        /* volta peca capturada */
        if(mi.capturou) {
            insertPeca(m.d, mi.p.type, mi.p.owner);
        }

        --move_count;

        ASSERT(checkBoard());
    }

    MoveInfo move_null() {
        MoveInfo mi;

        /* historico */
        history[move_count ++] = hash_key;

        /* marca coluna de en passan */
        mi.passan_place = passan_place;
        passan_place = 64;

        /* troca a vez */
        turn = OPPONENT(turn);
        hash_key ^= hash_code_turn;

        return mi;
    }

    void volta_null(const MoveInfo& mi) {
        /* volta coluna de en passan */
        passan_place = mi.passan_place;

        /* troca a vez */
        turn = OPPONENT(turn);
        hash_key ^= hash_code_turn;

        /* historico */
        move_count --;
    }

    bool validatePawn(const Move& m) {
        BitBoard blockers = this->getBlockers();
        if(this->passan_place != 64) blockers.set(this->passan_place);
        BitBoard dest = get_pawn_moves(b[m.o].owner, m.o, blockers) & ~(this->bb_blockers[b[m.o].owner]);
        return dest.is_set(m.d);
    }

    bool validateKnight(const Move& m) {
        BitBoard dest = get_knight_moves(m.o) & ~(this->bb_blockers[b[m.o].owner]);
        return dest.is_set(m.d);
    }

    bool validateBishop(const Move& m) {
        BitBoard dest = get_bishop_moves(m.o, this->getBlockers()) & ~(this->bb_blockers[b[m.o].owner]);
        return dest.is_set(m.d);
    }

    bool validateKing(const Move& m) {
        BitBoard dest = get_king_moves(m.o) & ~(this->bb_blockers[b[m.o].owner]);
        return dest.is_set(m.d);
    }

    bool validateRook(Move m) {
        BitBoard dest = get_rook_moves(m.o, this->getBlockers()) & ~(this->bb_blockers[b[m.o].owner]);
        return dest.is_set(m.d);
    }

    bool validateQueen(const Move& m) {
        BitBoard dest = get_queen_moves(m.o, this->getBlockers()) & ~(this->bb_blockers[b[m.o].owner]);
        return dest.is_set(m.d);
    }

    bool validateMove(const Move& m) {
        bool valid = true;
        int player = this->turn;
        /* verifica a vez */
        if(b[m.o].owner != player) {
            return false;
        }
        /* verifica jogada sem sentido */
        if(m.o == m.d) {
            return false;
        }

        /* valida o movimento da peça */
        switch(b[m.o].type) {
            case PAWN:
                valid = validatePawn(m);
                break;
            case KNIGHT:
                valid = validateKnight(m);
                break;
            case BISHOP:
                valid = validateBishop(m);
                break;
            case ROOK:
                valid = validateRook(m);
                break ;
            case QUEEN:
                valid =  validateQueen(m);
                break;
            case KING:
                valid = validateKing(m);
                break;
            default:
                /* Isso não deve acontecer */
                exit(1);
        }
        if(not valid) {
            return false;
        }

        /* verifica check */
        MoveInfo mi = this->move(m);
        valid = not this->checkCheck(player);
        this->volta(m, mi);

        return valid;
    }

    std::string getFen() {
        const char tabela[3][7]=
           {{'1','p','n','b','r','q','k'},
            {'1','P','N','B','R','Q','K'},
            {'1','1','1','1','1','1','1'}};

        std::string fen;
        for(int l=7;l>=0;--l) {
            for(int c=0;c<8;++c) {
                fen += tabela[b[make_place(l,c)].owner][b[make_place(l,c)].type];
            }
            fen += '/';
        }

        fen.erase(fen.end()-1);

        fen += ' ';
        fen += (turn == WHITE) ? 'w' : 'b';

        return fen;
    }

    void print(FILE * f)
    {
        const char tabela[7]={' ','P','C','B','T','D','R'};
        const char tabela2[3]={'P','B',' '};
        std::string linha;
        linha+='.';
        linha+='-';
        linha+='-';
        for(int i=1;i<8;i++) {
            linha+="---";
        }
        linha+='.';
        fprintf(f,"%s\n",linha.c_str());
        linha.clear();
        for(int l=7;;l--) {
            for(int c=0;c<8;c++) {
                linha+='|';
                if(b[make_place(l,c)].type == CLEAR) {
                    linha+="  ";
                } else {
                    linha+=tabela[(int)b[make_place(l,c)].type];
                    linha+=tabela2[b[make_place(l,c)].owner];
                }
            }
            linha+='|';
            fprintf(f,"%s\n",linha.c_str());
            linha.clear();
            linha += "|--";
            if(l==0)
                break;
            for(int i=1;i<8;i++) {
                linha+='+';
                linha+="--";
            }
            linha+='|';
            fprintf(f,"%s\n",linha.c_str());
            linha.clear();
        }

        linha+="*--";
        for(int i=1;i<8;i++) {
            linha+="---";
        }
        linha += '*';
        fprintf(f,"%s\n",linha.c_str());
        fprintf(f,"%s\n",getFen().c_str());
    }


    bool checkBoard()
    {
        for(int l=0;l<8;++l) {
            for(int c=0;c<8;++c) {
                int place = make_place(l, c);
                int type = b[place].type, owner = b[place].owner, id = b[place].id;
                if((this->bb_blockers[0].is_set(place) and (owner != BLACK)) or
                        (not this->bb_blockers[0].is_set(place) and (owner == BLACK))) {
                    fprintf(log_file, "blockers bitboard ta errado em (%d,%d) %d\n", l, c, place);
                    print_bitboard(log_file, this->getBlockers());
                    goto erro;
                }
                if(type == CLEAR and owner != NONE) {
                    fprintf(log_file, "%d %d dono de posicao vazia\n", l, c);
                    print(log_file);
                    goto erro;
                }
                if(type != CLEAR and owner == NONE) {
                    fprintf(log_file, "%d %d peca sem dono\n", l, c);
                    print(log_file);
                    goto erro;
                }
                if(type == CLEAR)
                    continue;
                if(peca_table[owner][type][id].place != place) {
                    fprintf(log_file, "2 - peca_table nao bate\n");
                    fprintf(log_file, "peca_table[%d][%d][%d] = (l=%d,c=%d) b = (type=%d,owner=%d,id=%d)\n",owner,type,id, l, c, type,owner,id);
                    print(log_file);
                    goto erro;
                }
            }
        }
        for(int owner=0;owner<2;++owner) {
            for(int type=1;type<7;++type) {
                for(int i=0;i<size_table[owner][type]; ++i) {
                    if(b[peca_table[owner][type][i].place].owner != owner or
                            b[peca_table[owner][type][i].place].type != type or
                            b[peca_table[owner][type][i].place].id != i) {
                        int l = peca_table[owner][type][i].place/8;
                        int c = peca_table[owner][type][i].place%8;
                        int place = peca_table[owner][type][i].place;
                        fprintf(log_file, "1 - peca_table nao bate\n");
                        fprintf(log_file, "peca_table[%d][%d][%d] = (l=%d,c=%d) b(type=%d,owner=%d,id=%d)\n",owner, type, i, l, c, b[place].type,b[place].owner,b[place].id);
                        print(log_file);
                        print_bitboard(log_file, this->getBlockers());
                        goto erro;
                    }
                }
            }
        }

        return true;
erro:
        fflush(log_file);
        return false;
    }

    bool checkAttack(int place, int player)
    {
        for(int i=0;i<size_table[player][PAWN];++i) {
            if(validatePawn(Move(peca_table[player][PAWN][i].place, place)))
                return true;
        }
        for(int i=0;i<size_table[player][KNIGHT];++i) {
            if(validateKnight(Move(peca_table[player][KNIGHT][i].place, place)))
                return true;
        }
        for(int i=0;i<size_table[player][BISHOP];++i) {
            if(validateBishop(Move(peca_table[player][BISHOP][i].place, place)))
                return true;
        }
        for(int i=0;i<size_table[player][ROOK];++i) {
            if(validateRook(Move(peca_table[player][ROOK][i].place, place)))
                return true;
        }
        for(int i=0;i<size_table[player][QUEEN];++i) {
            if(validateQueen(Move(peca_table[player][QUEEN][i].place, place)))
                return true;
        }
        for(int i=0;i<size_table[player][KING];++i) {
            if(validateKing(Move(peca_table[player][KING][i].place, place)))
                return true;
        }
        return false;
    }

    bool checkCheck(int player)
    {
        return checkAttack(peca_table[player][KING][0].place, OPPONENT(player));
    }

    bool checkCheck() {
        return checkCheck(this->turn);
    }

    BitBoard getBlockers() const {
        return this->bb_blockers[WHITE] | this->bb_blockers[BLACK];
    }

    int popgMoves(int o, BitBoard b, Move* list) {
        int n = 0;
        while(not b.empty()) {
            int d = b.pop_place();
            list[n++] = Move(o, d);
        }
        return n;
    }

    bool checkPawnMove(const Move& m, int owner) const {
        int dir = (owner==WHITE)? 1 : -1;
        if(m.d-m.o == 8*dir) {
            /* uma casa para frente */
            return b[m.d].owner == NONE;
        } else if(m.d-m.o == 16*dir) {
            /* duas casas para frente */
            return b[m.d].owner == NONE and b[(m.d+m.o)/2].owner == NONE;
        } else if((m.oc() > 0 and m.ol()+dir==m.dl() and m.oc()-1==m.dc()) or ((m.oc() < 7 and m.ol()+dir==m.dl() and m.oc()+1==m.dc()))) {
            /* captura ou en passan */
            return b[m.d].owner == OPPONENT(owner) or m.d == passan_place;
        } else {
            return false;
        }
    }

    int listPawnMovesg(int color, int place, Move *list) {
        BitBoard blockers = this->getBlockers();
        if(this->passan_place != 64) blockers.set(this->passan_place);
        BitBoard dest = get_pawn_moves(color, place, blockers) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listPawnAttacksg(int color, int place, Move *list) {
        BitBoard blockers = this->getBlockers();
        if(this->passan_place < 64) blockers.set(this->passan_place);
        BitBoard dest = get_pawn_capture_promotion_moves(color, place, blockers) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listKnightMovesg(int color, int place, Move *list) {
        BitBoard dest = get_knight_moves(place) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listKnightAttacksg(int color, int place, Move *list) {
        BitBoard dest = get_knight_moves(place) & this->bb_blockers[OPPONENT(color)];
        return popgMoves(place, dest, list);
    }

    int countKnightMovesg(int color, int place) {
        BitBoard dest = get_knight_moves(place) & ~(this->bb_blockers[color]);
        return dest.pop_count();
    }

    int listRookMovesg(int color, int place, Move *list) {
        BitBoard dest = get_rook_moves(place, this->getBlockers()) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listRookAttacksg(int color, int place, Move *list) {
        BitBoard dest = get_rook_moves(place, this->getBlockers()) & this->bb_blockers[OPPONENT(color)];
        return popgMoves(place, dest, list);
    }

    int countRookMovesg(int color, int place) {
        BitBoard block = ((this->bb_blockers[color] ^ this->bbPeca[color][ROOK]) ^ this->bbPeca[color][QUEEN]);
        //BitBoard block = this->bb_blockers[color];
        BitBoard dest = get_rook_moves(place, block | this->bb_blockers[OPPONENT(color)]) & ~(block);
        return dest.pop_count();
    }

    int listBishopMovesg(int color, int place, Move *list) {
        BitBoard dest = get_bishop_moves(place, this->getBlockers()) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listBishopAttacksg(int color, int place, Move *list) {
        BitBoard dest = get_bishop_moves(place, this->getBlockers()) & this->bb_blockers[OPPONENT(color)];
        return popgMoves(place, dest, list);
    }

    int countBishopMovesg(int color, int place) {
        BitBoard block = ((this->bb_blockers[color] ^ this->bbPeca[color][BISHOP]) ^ this->bbPeca[color][QUEEN]);
        //BitBoard block = (this->bb_blockers[color]);
        BitBoard dest = get_bishop_moves(place, block | this->bb_blockers[OPPONENT(color)]) & ~(block);
        return dest.pop_count();
    }

    int listKingMovesg(int color, int place, Move *list) {
        BitBoard dest = get_king_moves(place) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listKingAttacksg(int color, int place, Move *list) {
        BitBoard dest = get_king_moves(place) & this->bb_blockers[OPPONENT(color)];
        return popgMoves(place, dest, list);
    }

    int listQueenMovesg(int color, int place, Move *list) {
        BitBoard dest = get_queen_moves(place, this->getBlockers()) & ~(this->bb_blockers[color]);
        return popgMoves(place, dest, list);
    }

    int listQueenAttacksg(int color, int place, Move *list) {
        BitBoard dest = get_queen_moves(place, this->getBlockers()) & this->bb_blockers[OPPONENT(color)];
        return popgMoves(place, dest, list);
    }

    NOINLINE int countQueenMovesg(int color, int place, Move *list) {
        BitBoard block = (((this->bb_blockers[color] ^ this->bbPeca[color][ROOK]) ^ this->bbPeca[color][QUEEN]) ^ this->bbPeca[color][BISHOP]);
        BitBoard dest = get_queen_moves(place, block | this->bb_blockers[OPPONENT(color)]) & ~(block);
        return dest.pop_count();
    }

    NOINLINE int listMovesg(Move *list, int t)
    {
        int n = 0;

        for(int i=0;i<size_table[t][PAWN];++i) {
            n += listPawnMovesg(t, peca_table[t][PAWN][i].place, list + n);
        }

        for(int i=0;i<size_table[t][KNIGHT];++i) {
            n += listKnightMovesg(t, peca_table[t][KNIGHT][i].place, list + n);
        }

        for(int i=0;i<size_table[t][BISHOP];++i) {
            n += listBishopMovesg(t, peca_table[t][BISHOP][i].place, list + n);
        }

        for(int i=0;i<size_table[t][ROOK];++i) {
            n += listRookMovesg(t, peca_table[t][ROOK][i].place, list + n);
        }

        for(int i=0;i<size_table[t][QUEEN];++i) {
            n += listQueenMovesg(t, peca_table[t][QUEEN][i].place, list + n);
        }

        for(int i=0;i<size_table[t][KING];++i) {
            n += listKingMovesg(t, peca_table[t][KING][i].place, list + n);
        }

        return n;
    }

    NOINLINE int listAttacksg(Move *list, int t)
    {
        int n = 0;

        for(int i=0;i<size_table[t][PAWN];++i) {
            n += listPawnAttacksg(t, peca_table[t][PAWN][i].place, list + n);
        }

        for(int i=0;i<size_table[t][KNIGHT];++i) {
            n += listKnightAttacksg(t, peca_table[t][KNIGHT][i].place, list + n);
        }

        for(int i=0;i<size_table[t][BISHOP];++i) {
            n += listBishopAttacksg(t, peca_table[t][BISHOP][i].place, list + n);
        }

        for(int i=0;i<size_table[t][ROOK];++i) {
            n += listRookAttacksg(t, peca_table[t][ROOK][i].place, list + n);
        }

        for(int i=0;i<size_table[t][QUEEN];++i) {
            n += listQueenAttacksg(t, peca_table[t][QUEEN][i].place, list + n);
        }

        for(int i=0;i<size_table[t][KING];++i) {
            n += listKingAttacksg(t, peca_table[t][KING][i].place, list + n);
        }

        return n;
    }

    bool done() {
        Move moves[1024];
        MoveInfo mi;
        int n = listMovesg(moves, turn);
        bool res = true;
        int k = peca_table[turn][KING][0].place;

        for(int i=0;i<n;++i) {
            mi = move(moves[i]);
            if(k == moves[i].o) {
                if(not checkAttack(moves[i].d, turn)) {
                    res = false;
                }
            } else {
                if(not checkAttack(k, turn)) {
                    res = false;
                }
            }
            volta(moves[i], mi);
        }
        if(res == false)
            return false;
        if(checkCheck()) {
            result = (turn == BLACK)?"1-0 {mate}":"0-1 {mate}";
        } else {
            result = "1/2-1/2 {stalemate}";
        }
        return true;
    }

    NOINLINE int eval_pawns() {
        int pawn_score = 0;

        for(int t=0;t<2;++t) {
            int op = OPPONENT(t);
            for(int i=0;i<size_table[t][PAWN];++i) {
                int p = peca_table[t][PAWN][i].place;
                if((get_passed_pawn_mask(t, p) & bbPeca[op][PAWN]) == 0) {
                    pawn_score += passed_pawn_score[t][get_lin(p)];
                }
                if((get_neighbor_col_mask(p) & bbPeca[t][PAWN]) == 0) {
                    pawn_score += isolated_pawn_score[t];
                }
                if(((get_col_mask(p) & bbPeca[t][PAWN]).invert(p)) != 0) {
                    pawn_score += doubled_pawn_score[t];
                }
            }
        }

        return pawn_score;
    }

    NOINLINE int eval() {
        int score;
        int pawn_score = 0;
        int mob_score = 0;

        /*eval_hash_slot* slot;
        if((slot = eval_hash_find(hash_key, turn)) != 0) {
            return slot->score;
        }*/
        

        /* estrutura de peões */
        pawn_score = eval_pawns();

        /* mobilidade das pecas */
        for(int t=0;t<2;++t) {
            for(int i=0;i<size_table[t][KNIGHT];++i) {
                mob_score += mobility_score[t][KNIGHT] * countKnightMovesg(t, peca_table[t][KNIGHT][i].place);
            }
            for(int i=0;i<size_table[t][BISHOP];++i) {
                mob_score += mobility_score[t][BISHOP] * countBishopMovesg(t, peca_table[t][BISHOP][i].place);
            }
            for(int i=0;i<size_table[t][ROOK];++i) {
                mob_score += mobility_score[t][ROOK] * countRookMovesg(t, peca_table[t][ROOK][i].place);
            }
        }
        
        
        /* material score */
        if(turn == WHITE) {
            score = this->score + pawn_score + mob_score;
        } else {
            score = -(this->score + pawn_score + mob_score);
        }

        /*eval_hash_insert(hash_key, turn, score);*/

        return score;
    }

    bool check_hash_key(int linha) {
        uint64_t hashk = 0;
        for(int p=0;p<64;++p) {
            if(b[p].owner != NONE) {
                hashk ^= hash_code[p][b[p].type][b[p].owner];
            }
        }
        if(hashk != hash_key) {
            fprintf(log_file, "%d: Wrong hash_key\n", linha);
            fprintf(log_file, "%d: expected %llu got %llu\n", linha, hashk, hash_key);
            return false;
        }
        return true;
    }

    bool repeated() {
        int conta = 0;
        for(int i=0;i<move_count;++i) {
            if(history[i] == hash_key) {
                ++ conta;
                if(conta == 1) {
                    return true;
                }
            }
        }
        return false;
    }
} board;

Move parse_move_string(std::string move_str) {
    Move resp;
    if(move_str == "o-o-o" or move_str == "O-O-O" or move_str == "0-0-0") {
        if(board.turn == WHITE)
            move_str = "e1c1";
        else
            move_str = "e8c8";
    }
    if(move_str == "o-o" or move_str == "O-O" or move_str == "0-0") {
        if(board.turn == WHITE)
            move_str = "e1g1";
        else
            move_str = "e8g8";
    }
    resp = Move(move_str[1]-'1', move_str[0]-'a', move_str[3]-'1', move_str[2]-'a');
    return resp;
}

#endif
