#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <stdint.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


using namespace std;

#define CLEAR 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6
#define BLACK 0
#define WHITE 1  
#define NONE 2  
#define CASTLE 2
#define vl(x) ((x)<0 and (x)>7)
#define put(y,x) {list[n].ol=l;list[n].oc=c;list[n].dl=(y);list[n].dc=(x);n++;}
#define OPPONENT(p) ((p)^1)
#define VALID_POS(l,c) ((l)>=0 and (l)<8 and (c)>=0 and (c)<8)

#define HASH_UNUSED 0
#define HASH_UPPERBOUND 2
#define HASH_LOWERBOUND 3
#define HASH_EXACT 4

#define dificuldade 6

#ifdef _DEBUG
  #define SUICIDE() { int* __i__ = 0; int __v__ = *__i__; *__i__ = __v__;}
  #define ASSERT(VALUE) while(not (VALUE)) { SUICIDE(); }
#else
  #define SUICIDE()
  #define ASSERT(VALUE)
#endif

FILE* log_file = NULL;

struct Move
{
    Move(int8_t ol, int8_t oc, int8_t dl, int8_t dc) :
        ol(ol), oc(oc), dl(dl), dc(dc) { }
    Move() { }
    int8_t ol,oc,dl,dc;
    inline bool operator==(const Move& m) const {
        return ol==m.ol and dl==m.dl and oc==m.oc and dc==m.dc;
    }
    inline bool operator!=(const Move& m) const {
        return ol!=m.ol or dl!=m.dl or oc!=m.oc or dc!=m.dc;
    }
    inline bool is_valid() const {
        return VALID_POS(ol, oc) and VALID_POS(dl, dc) and (ol != dl or oc != dc);
    }
};

struct _peca
{
    int8_t l,c;
    int8_t type;
    int8_t owner;
};

struct _coord {
    int8_t l, c;
};

struct _pos
{
    _pos() { }

    int8_t id;
    int8_t id2;
    int8_t type;
    int8_t owner;
};

struct MoveInfo {
    MoveInfo() { }

    int8_t castle;
    bool fez_passan;
    bool capturou;
    bool promoveu;
    int8_t col_passan;
    _pos p;
};

const int mate_score = 100000;

const int material_table[3][7]={{   0,-100,-300,-300,-500,-900, -mate_score },
                                {   0, 100, 300, 300, 500, 900,  mate_score },
                                {   0,   0,   0,   0,   0,   0,  0          }};

const int dir[2] = {-1,1};

const int doubled_pawn_score[][7] = {{0,0, 35, 105, 155, 205, 255},
                                     {0,0,-35,-105,-155,-205,-255}};

const int isolated_pawn_score[2] = { 55,-55};

const int passed_pawn_score[2][8] = {{ 0,-50,-55,-61,-68,-76, 0},
                                     { 0, 50, 55, 61, 68, 76, 0}};


const int mobility_score[2][7] = {{ 0, 0,-4,-3,-2,-0, 0},
                                  { 0, 0, 4, 3, 2, 0, 0}};

uint64_t hash_code[8][8][2][8];
uint64_t hash_code_turn;

boost::mt19937 rng;
boost::uniform_int<uint32_t> uint32_interval(0,0xffffffffu);
boost::uniform_int<uint64_t> uint64_interval(0,0xffffffffffffffffllu);

boost::variate_generator<boost::mt19937&, boost::uniform_int<uint32_t> > rand32(rng, uint32_interval);
boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t> > rand64(rng, uint64_interval);

struct hash_slot {
    uint64_t hash_key;
    int32_t score;
    uint8_t depth;
    uint8_t type;
    Move move;

    /* debug */
    /*board_code code;
    uint64_t order;
    int alfa, beta;*/
};

struct eval_hash_slot {
    uint64_t hash_key;
    int32_t score;
};

static const unsigned long prime_list[] = {
    53ul,         97ul,         193ul,        389ul,       769ul,
    1543ul,       3079ul,       6151ul,       12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,     393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,    12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul,  402653189ul, 805306457ul,
    1610612741ul, 3221225473ul, 4294967291ul
};

const uint32_t HASH_SIZE = 786433;

hash_slot hash_table[2][HASH_SIZE];
eval_hash_slot eval_hash_table[2][HASH_SIZE];

void init_hash() {
    for(int l=0;l<8;++l) {
        for(int c=0;c<8;++c) {
            for(int owner=0;owner<2;++owner) {
                for(int t=0;t<8;++t) {
                    hash_code[l][c][owner][t] = rand64();
                }
            }
        }
    }
    for(int turn=0;turn<2;++turn) {
        for(uint32_t i=0;i<HASH_SIZE;++i) {
            hash_table[turn][i].hash_key = i+1;
            hash_table[turn][i].type = HASH_UNUSED;
            eval_hash_table[turn][i].hash_key = i+1;
        }
    }
    hash_code_turn = rand64();
}

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

void print_move(FILE* file, const Move& m) {
    fprintf(file,"%c%c%c%c", m.oc+'a',m.ol+'1',m.dc+'a',m.dl+'1');
}

struct Board
{
    _pos b[8][8];
    _peca peca[32];
    _coord peca_table[2][8][16];
    int8_t size_table[2][8];
    int8_t npeca;
    int8_t turn;
    int8_t col_passan;
    int32_t score;
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
        col_passan = -2;
        npeca = 0;
        memset(size_table, 0, sizeof(size_table));
        turn = WHITE;
        score = 0;
        hash_key = 0;
        for(int l=0;l<8;++l)
            for(int c=0;c<8;++c)
                b[l][c].owner = NONE;
        move_count = 0;
    }

    void setFen(const string& fen)
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
                insertPeca(linha, coluna, type, owner);
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

    void erasePeca1(int l, int c) {
        _pos p = b[l][c];
        if(p.id != npeca - 1) {
            peca[p.id] = peca[npeca - 1];
            b[peca[p.id].l][peca[p.id].c].id = p.id;
        }
        npeca--;
    }

    void erasePeca2(int l, int c) {
        _pos p = b[l][c];
        int tmp = size_table[p.owner][p.type]--;
        if(p.id2 != tmp - 1) {
            peca_table[p.owner][p.type][p.id2] = peca_table[p.owner][p.type][tmp-1];
            b[peca_table[p.owner][p.type][p.id2].l][peca_table[p.owner][p.type][p.id2].c].id2 = p.id2;
        }
    }

    void erasePeca(int l, int c) {
        score -= material_table[b[l][c].owner][b[l][c].type];

        hash_key ^= hash_code[l][c][b[l][c].owner][b[l][c].type];

        erasePeca1(l, c);
        erasePeca2(l, c);
        b[l][c].type = CLEAR;
        b[l][c].owner = NONE;
    }

    void insertPeca1(int l, int c, int type, int owner) {
        b[l][c].id = npeca;
        peca[npeca].l = l;
        peca[npeca].c = c;
        peca[npeca].type = type;
        peca[npeca].owner = owner;
        npeca++;
    }

    void insertPeca2(int l, int c, int type, int owner) {
        int tmp = size_table[owner][type]++;
        peca_table[owner][type][tmp].l = l;
        peca_table[owner][type][tmp].c = c;
        b[l][c].id2 = tmp;
    }

    void insertPeca(int l, int c, int type, int owner) {
        score += material_table[owner][type];

        hash_key ^= hash_code[l][c][owner][type];

        insertPeca1(l, c, type, owner);
        insertPeca2(l, c, type, owner);
        b[l][c].type = type;
        b[l][c].owner = owner;
    }

    void movePeca(int ol,int oc,int dl,int dc) {
        _pos s;

        hash_key ^= hash_code[ol][oc][b[ol][oc].owner][b[ol][oc].type];
        hash_key ^= hash_code[dl][dc][b[ol][oc].owner][b[ol][oc].type];

        s = b[ol][oc];
        peca[s.id].l=dl;
        peca[s.id].c=dc;
        peca_table[s.owner][s.type][s.id2].l=dl;
        peca_table[s.owner][s.type][s.id2].c=dc;
        b[dl][dc] = b[ol][oc];
        b[ol][oc].type = CLEAR;
        b[ol][oc].owner = NONE;

    }

    void setType(int l, int c, int t) {
        score += material_table[b[l][c].owner][t] - material_table[b[l][c].owner][b[l][c].type];

        hash_key ^= hash_code[l][c][b[l][c].owner][t];
        hash_key ^= hash_code[l][c][b[l][c].owner][b[l][c].type];

        peca[b[l][c].id].type = t;
        erasePeca2(l, c);
        insertPeca2(l, c, t, b[l][c].owner);
        b[l][c].type = t;
    }

    MoveInfo move(Move m) {
        MoveInfo mi;

        ASSERT(checkBoard());

        history[move_count ++] = hash_key;

        /* copia peca capturada */
        if(b[m.dl][m.dc].type != CLEAR) {
            mi.p = b[m.dl][m.dc];
            mi.capturou = true;
            erasePeca(m.dl, m.dc);
        } else {
            mi.capturou = false;
        }

        /* movimenta a peca */
        movePeca(m.ol, m.oc, m.dl, m.dc);

        /* promove */
        if(b[m.dl][m.dc].type == PAWN and (m.dl==7 or m.dl==0)) {
            setType(m.dl, m.dc, QUEEN);
            mi.promoveu = true;
        } else {
            mi.promoveu = false;
        }

        /* marca coluna de en passan */
        mi.col_passan = col_passan;
        if(b[m.dl][m.dc].type == PAWN and (m.dl-m.ol==2 or m.dl-m.ol==-2)) {
            col_passan = m.oc;
        } else {
            col_passan = -2;
        }

        /* faz en passan */
        if(b[m.dl][m.dc].type == PAWN and (not mi.capturou) and m.dc != m.oc) {
            mi.p = b[m.ol][m.dc];
            mi.fez_passan = true;
            erasePeca(m.ol, m.dc);
        } else {
            mi.fez_passan = false;
        }

        /* faz roque */
        mi.castle = 0;
        if(b[m.dl][m.dc].type == KING) {
            if((m.dc-m.oc)==2) {
                movePeca(m.dl,7,m.dl,5);
                mi.castle = 1;
            } else if((m.dc-m.oc)==-2) {
                movePeca(m.dl,0,m.dl,3);
                mi.castle = 1;
            }
        }

        /* troca a vez */
        turn = OPPONENT(turn);
        hash_key ^= hash_code_turn;

        ASSERT(checkBoard());

        return mi;
    }

    MoveInfo move_null() {
        MoveInfo mi;

        /* historico */
        history[move_count ++] = hash_key;

        /* marca coluna de en passan */
        mi.col_passan = col_passan;
        col_passan = -2;

        /* troca a vez */
        turn = OPPONENT(turn);
        hash_key ^= hash_code_turn;

        return mi;
    }

    void volta_null(const MoveInfo& mi) {
        /* volta coluna de en passan */
        col_passan = mi.col_passan;

        /* troca a vez */
        turn = OPPONENT(turn);
        hash_key ^= hash_code_turn;

        /* historico */
        move_count --;
    }

    void volta(Move m, const MoveInfo& mi) {

        ASSERT(checkBoard());

        /* troca a vez */
        hash_key ^= hash_code_turn;
        turn = OPPONENT(turn);

        /* desfaz roque */
        if(mi.castle) {
            if((m.dc-m.oc)==2) {
                movePeca(m.dl,5,m.dl,7);
            } else if((m.dc-m.oc)==-2) {
                movePeca(m.dl,3,m.dl,0);
            }
        }

        /* desfaz promocao */
        if(mi.promoveu) {
            setType(m.dl, m.dc, PAWN);
        }

        /* desfaz passan */
        if(mi.fez_passan) {
            insertPeca(m.ol, m.dc, mi.p.type, mi.p.owner);
        }

        /* volta coluna de en passan */
        col_passan = mi.col_passan;

        /* volta a peca */
        movePeca(m.dl, m.dc, m.ol, m.oc);

        /* volta peca capturada */
        if(mi.capturou) {
            insertPeca(m.dl, m.dc, mi.p.type, mi.p.owner);
        }

        --move_count;

        ASSERT(checkBoard());
    }


    bool validatePawn(const Move& m) {
        if(b[m.ol][m.oc].owner == WHITE) {
            if(m.dc == m.oc) {
                /* movimento para frente */
                if(m.dl - m.ol == 2) {
                    return m.ol == 1 and b[2][m.oc].type == CLEAR and b[3][m.oc].type == CLEAR;
                } else if(m.dl - m.ol == 1) {
                    return b[m.dl][m.dc].type == CLEAR;
                } else {
                    return false;
                }
            } else if((m.oc - m.dc == 1 or m.oc - m.dc == -1) and (m.dl - m.ol == 1)) {
                /* captura e en passan */
                return b[m.dl][m.dc].owner == BLACK or (m.ol == 4 and m.dc == col_passan);
            } else {
                return false;
            }
        } else {
            if(m.dc == m.oc) {
                /* movimento para frente */
                if(m.dl - m.ol == -2) {
                    return m.ol == 6 and b[5][m.oc].type == CLEAR and b[4][m.oc].type == CLEAR;
                } else if(m.dl - m.ol == -1) {
                    return b[m.dl][m.dc].type == CLEAR;
                } else {
                    return false;
                }
            } else if((m.oc - m.dc == 1 or m.oc - m.dc == -1) and (m.dl - m.ol == -1)) {
                /* captura e en passan */
                return b[m.dl][m.dc].owner == WHITE or (m.ol == 3 and m.dc == col_passan);
            } else {
                return false;
            }
        }
    }

    bool validateKnight(const Move& m) {
        int t = (m.ol-m.dl)*(m.oc-m.dc);
        return (t == 2 or t == -2) and (b[m.dl][m.dc].owner != b[m.ol][m.oc].owner);
    }

    bool validateBishop(const Move& m) {
        int di, dj;
        if(abs(m.oc-m.dc) != abs(m.ol-m.dl))
            return false;
        di = (m.dc>m.oc)?1:-1;
        dj = (m.dl>m.ol)?1:-1;
        for(int i = m.oc+di,j=m.ol+dj;i!=m.dc;i+=di, j+=dj)
            if(b[j][i].type != CLEAR)
                return false;
        return b[m.dl][m.dc].owner != b[m.ol][m.oc].owner;
    }

    bool validateKing(const Move& m) {
        if(m.oc-m.dc == 2 or m.oc-m.dc == -2) {
            return validateCastle(m);
        }
        if(m.ol-m.dl > 1 or m.ol-m.dl < -1 or m.oc-m.dc > 1 or m.oc-m.dc < -1)
            return false;
        return b[m.dl][m.dc].owner != b[m.ol][m.oc].owner;
    }

    bool validateRook(Move m) {
        int di, dj;
        if(!((m.oc!=m.dc) ^ (m.ol!=m.dl)))
            return false;
        di=dj=0;
        if(m.dc>m.oc)
            di=1;
        else if(m.dc<m.oc)
            di=-1;
        if(m.dl>m.ol)
            dj=1;
        else if(m.dl<m.ol)
            dj=-1;
        for(int i=m.oc+di,j=m.ol+dj;i!=m.dc or j!=m.dl;i+=di, j+=dj)
            if(b[j][i].type)
                return false;
        return b[m.dl][m.dc].owner != b[m.ol][m.oc].owner;;
    }

    bool validateQueen(const Move& m) {
        return validateBishop(m) or
            validateRook(m);
    }

    bool validateMove(const Move& m) {
        /* verifica a vez */
        if(b[m.ol][m.oc].owner != turn) {
            return false;
        }
        /* verifica jogada sem sentido */
        if(m.ol == m.dl and m.oc == m.dc) {
            return false;
        }
        switch(b[m.ol][m.oc].type) {
            case PAWN:
                return validatePawn(m);
            case KNIGHT:
                return validateKnight(m);
            case BISHOP:
                return validateBishop(m);
            case ROOK:
                return validateRook(m);
            case QUEEN:
                return validateQueen(m);
            case KING:
                return validateKing(m);
            default:
                /* Isso não deve acontecer */
                exit(1);
        }
    }

    std::string getFen() {
        const char tabela[3][7]=
           {{'1','p','n','b','r','q','k'},
            {'1','P','N','B','R','Q','K'},
            {'1','1','1','1','1','1','1'}};

        std::string fen;
        for(int l=7;l>=0;--l) {
            for(int c=0;c<8;++c) {
                fen += tabela[b[l][c].owner][b[l][c].type];
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

        int i;
        unsigned char linha[128];
        linha[0]='.';
        linha[1]='-';
        linha[2]='-';
        for(i=1;i<8;i++) {
            linha[3*i]='-';
            linha[3*i+1]='-';
            linha[3*i+2]='-';
        }
        linha[3*i]='.';
        linha[3*i+1]=0;
        fprintf(f,"%s\n",linha);
        for(int l=7;;l--) {
            for(i=0;i<8;i++) {
                linha[3*i]='|';
                if(b[l][i].type == CLEAR) {
                    linha[3*i+1]=linha[3*i+2]=' ';
                    continue;
                }
                linha[3*i+1]=tabela[(int)b[l][i].type];
                linha[3*i+2]=tabela2[b[l][i].owner];
            }
            linha[3*i]='|';
            linha[3*i+1]=0;
            fprintf(f,"%s\n",linha);
            linha[0]='|';
            linha[1]='-';
            linha[2]='-';
            if(l==0)
                break;
            for(i=1;i<8;i++) {
                linha[3*i]='+';
                linha[3*i+1]=linha[3*i+2]='-';
            }
            linha[3*i]='|';
            linha[3*i+1]=0;
            fprintf(f,"%s\n",linha);
        }
        linha[0]='*';
        linha[1]=linha[2]='-';
        for(int i=1;i<8;i++) {
            linha[3*i]='-';
            linha[3*i+1]=linha[3*i+2]='-';
        }
        linha[3*i]='*';
        linha[3*i+1]=0;
        fprintf(f,"%s\n",linha);
        fprintf(f,"%s\n",getFen().c_str());
    }


    bool checkBoard()
    {
        int i,j,n;
        for(i=0,n=0;i<8;i++) {
            for(j=0;j<8;j++) {
                if(b[i][j].type)
                    n++;
            }
        }
        if(n!=npeca) {
            fprintf(log_file,"Contagem nao bate\n");
            print(log_file);
            return false;
        }
        for(i=0;i<npeca;i++) {
            if(b[peca[i].l][peca[i].c].type!=peca[i].type or b[peca[i].l][peca[i].c].owner!=peca[i].owner or b[peca[i].l][peca[i].c].id!=i) {
                fprintf(log_file,"(%d,%d) -> peca(type=%d,owner=%d,id=%d) Board(type=%d,owner=%d,id=%d)\n",peca[i].l,peca[i].c,peca[i].type,peca[i].owner,i,b[peca[i].l][peca[i].c].type,b[peca[i].l][peca[i].c].owner,b[peca[i].l][peca[i].c].id);
                print(log_file);
                return false;
            }
        }
        for(int t=0;t<2;++t) {
            for(int p=1;p<7;++p) {
                for(int i=0;i<size_table[t][p]; ++i) {
                    if(b[peca_table[t][p][i].l][peca_table[t][p][i].c].owner != t or
                            b[peca_table[t][p][i].l][peca_table[t][p][i].c].type != p or
                            b[peca_table[t][p][i].l][peca_table[t][p][i].c].id2 != i) {
                        int l = peca_table[t][p][i].l;
                        int c = peca_table[t][p][i].c;
                        fprintf(log_file, "peca_table nao bate\n");
                        fprintf(log_file,"peca_table[%d][%d][%d] = (l=%d,c=%d) b(type=%d,owner=%d,id2=%d)\n",t, p, i, l, c, b[l][c].type,b[l][c].owner,b[l][c].id2);
                        print(log_file);
                        return false;
                    }
                }
            }
        }

        for(int l=0;l<8;++l) {
            for(int c=0;c<8;++c) {
                if(b[l][c].type == CLEAR and b[l][c].owner != NONE) {
                    fprintf(log_file, "%d %d dono de posicao vazia\n", l, c);
                    print(log_file);
                    return false;
                }
                if(b[l][c].type != CLEAR and b[l][c].owner == NONE) {
                    fprintf(log_file, "%d %d peca sem dono\n", l, c);
                    print(log_file);
                    return false;
                }
                if(b[l][c].type == CLEAR)
                    continue;
                int o = b[l][c].owner;
                if(peca_table[o][b[l][c].type][b[l][c].id2].l != l or
                        peca_table[o][b[l][c].type][b[l][c].id2].c != c) {
                    fprintf(log_file, "peca_table nao bate\n");
                    fprintf(log_file,"peca_table[%d][%d][%d] = (l=%d,c=%d) b = (type=%d,owner=%d,id2=%d)\n",o,b[l][c].type,b[l][c].id2, l, c, b[l][c].type,b[l][c].owner,b[l][c].id2);
                    print(log_file);
                    return false;
                }
                if(peca[b[l][c].id].type != b[l][c].type or
                        peca[b[l][c].id].l != l or
                        peca[b[l][c].id].c != c or
                        peca[b[l][c].id].owner != b[l][c].owner) {
                    fprintf(log_file,"peca[%d] = (type=%d,owner=%d) b[%d][%d] = (type=%d,owner=%d,id=%d)\n",b[l][c].id, peca[b[l][c].id].type,peca[b[l][c].type].owner, l, c, b[l][c].type, b[l][c].owner, b[l][c].id);
                    print(log_file);
                    return false;
                }
            }
        }
        return true;
    }

    bool checkAttack(int l, int c, int player)
    {
        for(int i=0;i<size_table[player][PAWN];++i) {
            if(validatePawn(Move(peca_table[player][PAWN][i].l, peca_table[player][PAWN][i].c, l, c)))
                return true;
        }
        for(int i=0;i<size_table[player][KNIGHT];++i) {
            if(validateKnight(Move(peca_table[player][KNIGHT][i].l, peca_table[player][KNIGHT][i].c, l, c)))
                return true;
        }
        for(int i=0;i<size_table[player][BISHOP];++i) {
            if(validateBishop(Move(peca_table[player][BISHOP][i].l, peca_table[player][BISHOP][i].c, l, c)))
                return true;
        }
        for(int i=0;i<size_table[player][ROOK];++i) {
            if(validateRook(Move(peca_table[player][ROOK][i].l, peca_table[player][ROOK][i].c, l, c)))
                return true;
        }
        for(int i=0;i<size_table[player][QUEEN];++i) {
            if(validateQueen(Move(peca_table[player][QUEEN][i].l, peca_table[player][QUEEN][i].c, l, c)))
                return true;
        }
        for(int i=0;i<size_table[player][KING];++i) {
            if(validateKing(Move(peca_table[player][KING][i].l, peca_table[player][KING][i].c, l, c)))
                return true;
        }
        return false;
    }

    bool validateCastle(const Move& m)
    {
        int torre;
        if(m.dc-m.oc == 2)
            torre = 7;
        else if(m.dc-m.oc == -2)
            torre = 0;
        else
            return false;
        if((b[m.ol][m.oc].owner == WHITE and m.ol != 0) or (b[m.ol][m.oc].owner == BLACK and m.ol != 7))
            return false;
        if(b[m.ol][torre].type != ROOK or b[m.ol][m.oc].owner != b[m.ol][torre].owner)
            return false;
        if(b[m.dl][m.dc].type != CLEAR or b[m.dl][(m.dc+m.oc)/2].type != CLEAR)
            return false;
        int op = OPPONENT(b[m.ol][m.oc].owner);
        if(checkAttack(m.dl,m.dc,op) or checkAttack(m.dl,(m.dc+m.oc)/2,op) or checkAttack(m.ol,m.oc,op)) {
            return false;
        }
        return true;
    }

    bool checkCheck()
    {
        int i;
        int op = OPPONENT(turn);
        for(i=0;i<npeca;i++) {
            if(peca[i].type==KING and peca[i].owner==turn) {
                return checkAttack(peca[i].l,peca[i].c, op);
            }
        }
        return false;
    }

    int listWhitePawnMoves(int l, int c, Move *list) {
        int n = 0;

        if(b[l+1][c].type == CLEAR) {
            put(l+1,c);
            if(l == 1 and b[l+2][c].type == CLEAR) {
                put(l+2,c);
            }
        }
        if(c < 7 and b[l+1][c+1].owner == BLACK)
            put(l+1,c+1);
        if(c > 0 and b[l+1][c-1].owner == BLACK)
            put(l+1,c-1);
        if(l == 4 and (col_passan == c-1 or col_passan == c+1)) {
            put(l+1, col_passan);
        }

        return n;
    }

    int listBlackPawnMoves(int l, int c, Move *list) {
        int n = 0;

        if(b[l-1][c].type == CLEAR) {
            put(l-1,c);
            if(l == 6 and b[l-2][c].type == CLEAR) {
                put(l-2,c);
            }
        }
        if(c < 7 and b[l-1][c+1].owner == WHITE)
            put(l-1,c+1);
        if(c > 0 and b[l-1][c-1].owner == WHITE)
            put(l-1,c-1);
        if(l == 3 and (col_passan == c-1 or col_passan == c+1)) {
            put(l-1, col_passan);
        }

        return n;
    }

    int listKnightMoves(int l, int c, Move *list) {
        static const int dl[] = { 2, 1,-1,-2,-2,-1, 1, 2};
        static const int dc[] = { 1, 2, 2, 1,-1,-2,-2,-1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner != b[l][c].owner) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listRookMoves(int l, int c, Move *list) {
        static const int dl[] = { 1, 0,-1, 0};
        static const int dc[] = { 0, 1, 0,-1};
        int n = 0;
        int l1, c1;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]) {
                put(l1,c1);
            }
            if(VALID_POS(l1,c1) and b[l1][c1].owner != b[l][c].owner) {
                put(l1,c1);
            }
        }
        return n;
    }

    int listBishopMoves(int l, int c, Move *list) {
        static const int dl[] = {1, 1,-1,-1};
        static const int dc[] = {1,-1,-1, 1};
        int l1,c1;
        int n = 0;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]) {
                put(l1,c1);
            }
            if(VALID_POS(l1,c1) and b[l1][c1].owner != b[l][c].owner) {
                put(l1,c1);
            }
        }
        return n;
    }

    int listKingMoves(int l, int c, Move *list) {
        static const int dl[] = { 1, 1, 1, 0,-1,-1,-1, 0};
        static const int dc[] = { 1, 0,-1,-1,-1, 0, 1, 1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner != b[l][c].owner) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listQueenMoves(int l, int c, Move *list) {
        int n = 0;

        n += listRookMoves(l, c, list);
        n += listBishopMoves(l, c, list + n);

        return n;
    }

    __attribute__((noinline)) int listMoves(Move *list, int t)
    {
        int n = 0;

        if(t == WHITE) {
            for(int i=0;i<size_table[WHITE][PAWN];++i) {
                n += listWhitePawnMoves(peca_table[WHITE][PAWN][i].l, peca_table[WHITE][PAWN][i].c, list + n);
            }
        } else {
            for(int i=0;i<size_table[BLACK][PAWN];++i) {
                n += listBlackPawnMoves(peca_table[BLACK][PAWN][i].l, peca_table[BLACK][PAWN][i].c, list + n);
            }
        }

        for(int i=0;i<size_table[t][KNIGHT];++i) {
            n += listKnightMoves(peca_table[t][KNIGHT][i].l, peca_table[t][KNIGHT][i].c, list + n);
        }

        for(int i=0;i<size_table[t][BISHOP];++i) {
            n += listBishopMoves(peca_table[t][BISHOP][i].l, peca_table[t][BISHOP][i].c, list + n);
        }

        for(int i=0;i<size_table[t][ROOK];++i) {
            n += listRookMoves(peca_table[t][ROOK][i].l, peca_table[t][ROOK][i].c, list + n);
        }

        for(int i=0;i<size_table[t][QUEEN];++i) {
            n += listQueenMoves(peca_table[t][QUEEN][i].l, peca_table[t][QUEEN][i].c, list + n);
        }

        for(int i=0;i<size_table[t][KING];++i) {
            n += listKingMoves(peca_table[t][KING][i].l, peca_table[t][KING][i].c, list + n);
        }

        return n;
    }

    int listWhitePawnAttacks(int l, int c, int op, Move* list) {
        int n = 0;
        if(c>0 and b[l+1][c-1].owner == op)
            put(l+1,c-1);
        if(c<7 and b[l+1][c+1].owner == op)
            put(l+1,c+1);
        if(l==6 and b[l+1][c].owner == NONE)
            put(l+1,c);
        return n;
    }

    int listBlackPawnAttacks(int l, int c, int op, Move* list) {
        int n = 0;
        if(c>0 and b[l-1][c-1].owner == op)
            put(l-1,c-1);
        if(c<7 and b[l-1][c+1].owner == op)
            put(l-1,c+1);
        if(l==1 and b[l-1][c].owner == NONE)
            put(l-1,c);
        return n;
    }


    int listKnightAttacks(int l, int c, int op, Move* list) {
        static const int dl[] = { 2, 1,-1,-2,-2,-1, 1, 2};
        static const int dc[] = { 1, 2, 2, 1,-1,-2,-2,-1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner == op) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listBishopAttacks(int l, int c, int op, Move* list) {
        static const int dl[] = { 1, 1,-1,-1};
        static const int dc[] = { 1,-1,-1, 1};
        int l1,c1;
        int n = 0;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]);
            if(VALID_POS(l1,c1) and b[l1][c1].owner == op)
                put(l1,c1);
        }
        return n;
    }

    int listRookAttacks(int l, int c, int op, Move* list) {
        static const int dl[] = { 1, 0,-1, 0};
        static const int dc[] = { 0, 1, 0,-1};
        int n = 0;
        int l1, c1;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]);
            if(VALID_POS(l1,c1) and b[l1][c1].owner == op) {
                put(l1,c1);
            }
        }
        return n;
    }

    int listQueenAttacks(int l, int c, int op, Move* list) {
        int n = 0;
        n += listBishopAttacks(l, c, op, list);
        n += listRookAttacks(l, c, op, list + n);
        return n;
    }

    int listKingAttacks(int l, int c, int op, Move* list) {
        static const int dl[] = { 1, 1, 1, 0,-1,-1,-1, 0};
        static const int dc[] = { 1, 0,-1,-1,-1, 0, 1, 1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner == op) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listAttacks(Move* list) {

        int n = 0;
        int op = OPPONENT(turn);

        if(turn == WHITE) {
            for(int i=0;i<size_table[WHITE][PAWN];++i) {
                n += listWhitePawnAttacks(peca_table[WHITE][PAWN][i].l, peca_table[WHITE][PAWN][i].c, op, list + n);
            }
        } else {
            for(int i=0;i<size_table[BLACK][PAWN];++i) {
                n += listBlackPawnAttacks(peca_table[BLACK][PAWN][i].l, peca_table[BLACK][PAWN][i].c, op, list + n);
            }
        }

        for(int i=0;i<size_table[turn][KNIGHT];++i) {
            n += listKnightAttacks(peca_table[turn][KNIGHT][i].l, peca_table[turn][KNIGHT][i].c, op, list + n);
        }

        for(int i=0;i<size_table[turn][BISHOP];++i) {
            n += listBishopAttacks(peca_table[turn][BISHOP][i].l, peca_table[turn][BISHOP][i].c, op, list + n);
        }

        for(int i=0;i<size_table[turn][ROOK];++i) {
            n += listRookAttacks(peca_table[turn][ROOK][i].l, peca_table[turn][ROOK][i].c, op, list + n);
        }

        for(int i=0;i<size_table[turn][QUEEN];++i) {
            n += listQueenAttacks(peca_table[turn][QUEEN][i].l, peca_table[turn][QUEEN][i].c, op, list + n);
        }

        for(int i=0;i<size_table[turn][KING];++i) {
            n += listKingAttacks(peca_table[turn][KING][i].l, peca_table[turn][KING][i].c, op, list + n);
        }

        return n;
    }

    bool done() {
        Move moves[1024];
        MoveInfo mi;
        int kl = 0, kc = 0;
        int n = listMoves(moves, turn);
        bool res = true;

        kl = peca_table[turn][KING][0].l;
        kc = peca_table[turn][KING][0].c;

        for(int i=0;i<n;++i) {
            mi = move(moves[i]);
            if(kl == moves[i].ol and kc == moves[i].oc) {
                if(not checkAttack(moves[i].dl, moves[i].dc, turn)) {
                    res = false;
                }
            } else {
                if(not checkAttack(kl, kc, turn)) {
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

    int countKnightMoves(int l, int c) {
        static const int dl[] = { 2, 1,-1,-2,-2,-1, 1, 2};
        static const int dc[] = { 1, 2, 2, 1,-1,-2,-2,-1};

        int o = b[l][c].owner;
        int score = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner != o) {
                score ++;
            }
        }
        return score;
    }

    int countBishopMoves(int l, int c) {
        static const int dl[] = { 1, 1,-1,-1};
        static const int dc[] = { 1,-1,-1, 1};
        int l1,c1;
        int score = 0;
        int o = b[l][c].owner;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i]; VALID_POS(l1,c1) and (b[l1][c1].type == CLEAR or ((b[l1][c1].type == QUEEN or b[l1][c1].type == BISHOP) and b[l1][c1].owner == o));l1+=dl[i],c1+=dc[i]) {
                score ++;
            }
            if(VALID_POS(l1,c1) and b[l1][c1].owner != b[l][c].owner) {
                score ++;
            }
        }
        return score;
    }

    int countRookMoves(int l, int c) {
        static const int dl[] = { 1, 0,-1, 0};
        static const int dc[] = { 0, 1, 0,-1};
        int l1, c1;
        int score = 0;
        int o = b[l][c].owner;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and (b[l1][c1].type == CLEAR or ((b[l1][c1].type == ROOK or b[l1][c1].type == QUEEN) and b[l1][c1].owner == o));l1+=dl[i],c1+=dc[i]) {
                score ++;
            }
            if(VALID_POS(l1,c1) and b[l1][c1].owner != b[l][c].owner) {
                score ++;
            }
        }
        return score;
    }

    int countQueenMoves(int l, int c) {
        return countBishopMoves(l, c) + countRookMoves(l ,c);
    }

    __attribute__((noinline)) int32_t eval() {
        int32_t score;
        int pawn_score = 0;
        int mob_score = 0;
        int p[2][10];
        eval_hash_slot* slot;

        if((slot = eval_hash_find(hash_key, turn)) != 0) {
            return slot->score;
        }
        

        /* estrutura de peões */
        for(int i=0;i<10;++i) {
            p[0][i] = p[1][i] = 0;
        }
        for(int t=0;t<2;++t) {
            for(int i=0;i<size_table[t][PAWN];++i) {
                p[t][peca_table[t][PAWN][i].c+1]++;
            }
        }
        for(int t=0;t<2;++t) {
            int op = OPPONENT(t);
            for(int i=1;i<9;++i) {
                pawn_score += doubled_pawn_score[t][p[t][i]];
                if(p[t][i] > 0 and p[t][i-1] == 0 and p[t][i+1] == 0) {
                    pawn_score += isolated_pawn_score[t];
                }
            }
            for(int i=0;i<size_table[t][PAWN];++i) {
                int c = peca_table[t][PAWN][i].c+1;
                if(p[op][c] == 0 and p[op][c-1] == 0 and p[op][c+1] == 0) {
                    pawn_score += passed_pawn_score[t][peca_table[t][PAWN][i].l];
                }
            }
        }

        /* mobilidade das pecas */
        for(int t=0;t<2;++t) {
            for(int i=0;i<size_table[t][KNIGHT];++i) {
                mob_score += mobility_score[t][KNIGHT] * countKnightMoves(peca_table[t][KNIGHT][i].l, peca_table[t][KNIGHT][i].c);
            }
            for(int i=0;i<size_table[t][BISHOP];++i) {
                mob_score += mobility_score[t][BISHOP] * countBishopMoves(peca_table[t][BISHOP][i].l, peca_table[t][BISHOP][i].c);
            }
            for(int i=0;i<size_table[t][ROOK];++i) {
                mob_score += mobility_score[t][ROOK] * countRookMoves(peca_table[t][ROOK][i].l, peca_table[t][ROOK][i].c);
            }
        }
        
        
        /* material score */
        if(turn == WHITE) {
            score = this->score + pawn_score + mob_score;
        } else {
            score = -(this->score + pawn_score + mob_score);
        }

        eval_hash_insert(hash_key, turn, score);

        return score;
    }

    bool check_hash_key(int linha) {
        uint64_t hashk = 0;
        for(int l=0;l<8;++l) {
            for(int c=0;c<8;++c) {
                if(b[l][c].owner != NONE) {
                    hashk ^= hash_code[l][c][b[l][c].owner][b[l][c].type];
                }
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


const int encode_table[][7] = {{ 0, 1, 2, 3, 4, 5, 6},
                               { 0, 7, 8, 9,10,11,12},
                               { 0, 0, 0, 0, 0, 0, 0}};

struct board_code {
    uint32_t code[8];

    void encode() {
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

board_code enc() {
    board_code bc;
    bc.encode();
    return bc;
}

Move ComputerPlay(int depth, bool post);

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
    resp.oc = move_str[0]-'a';
    resp.ol = move_str[1]-'1';
    resp.dc = move_str[2]-'a';
    resp.dl = move_str[3]-'1';
    return resp;
}

Move list[256];

int random_play;

int main(int argc, char** argv)
{
    char comando[1024];
    char* args;
    int play = BLACK;
    int go = 0;
    int depth = dificuldade;
    int my_time = 0, op_time = 0;
    bool ponder = false, post = false;
    random_play = 0;

    if(argc==1)
        log_file = fopen("tmp.txt","w");
    else
        log_file = fopen(argv[1],"w");

    int seed = time(NULL);
    fprintf(log_file,"seed %d\n", seed);

    init_hash();
    srand48(seed);
    board.setInitial();
    ASSERT(board.checkBoard());
    board.check_hash_key(__LINE__);

    while(1) {
        if(fgets(comando, sizeof(comando), stdin) == NULL)
            break;
        fprintf(log_file, "-> %s", comando);
        comando[strlen(comando)-1]=0;
        fflush(log_file);
        int i;
        for(i = 0; comando[i]!=0 and !isspace(comando[i]);++i);
        if(comando[i]!=0) {
            comando[i]=0;
            ++i;
            for(;isspace(comando[i]);++i);
        }
        args = comando + i;

        if(strcmp(comando, "xboard")==0) {
            /* so uso o xabord mesmo, não faça nada aqui */
        } else if(strcmp(comando, "protover")==0) {
            /* features */
            /* não queremos nenhum sinal */
            printf("feature myname=\"raphael-chess%d\" ping=1 usermove=1 draw=0 variants=\"normal\" sigint=0 sigterm=0 setboard=1 playother=1 analyze=0 colors=0 done=1\n",dificuldade);
        } else if(strcmp(comando, "new")==0) {
            /* reinicia tudo */
            board.setInitial();
            play = BLACK;
            go = 1;
        } else if(strcmp(comando, "variant")==0) {
            /* não suportamos nenhuma variante */
        } else if(strcmp(comando, "quit")==0) {
            /* fecha */
            break;
        } else if(strcmp(comando, "random")==0) {
            /* coloca aleatoriedade nas jogadas */
            random_play = not random_play;
        } else if(strcmp(comando, "force")==0) {
            /* pare de jogar */
            go = 0;
        } else if(strcmp(comando, "go")==0) {
            /* jogue! */
            play = board.turn;
            go = 1;
        } else if(strcmp(comando, "playother")==0) {
            /* jogue com o outra cor */
            play = board.turn^1;
            go=1;
        } else if(strcmp(comando, "white")==0) {
            /* obsoleto, não faça nada */
        } else if(strcmp(comando, "black")==0) {
            /* obsoleto, não faça nada */
        } else if(strcmp(comando, "level")==0) {
        } else if(strcmp(comando, "st")==0) {
        } else if(strcmp(comando, "sd")==0) {
            /* profundidade limite */
            depth = atoi(args);
        } else if(strcmp(comando, "time")==0) {
            /* meu tempo */
            my_time = atoi(args);
        } else if(strcmp(comando, "otim")==0) {
            /* tempo do opoente */
            op_time = atoi(args);
        } else if(strcmp(comando, "board")==0) {
            /* extensão, imprime o tabuleiro */
            board.print(stdout);
        } else if(strcmp(comando, "ping")==0) {
            printf("pong %s\n", args);
        } else if(strcmp(comando, "draw")==0) {
        } else if(strcmp(comando, "result")==0) {
            /* temos um resultado, não jogue mais */
            go=0;
        } else if(strcmp(comando, "setboard")==0) {
            /* seta para a posição dada */
            board.setFen(args);
        } else if(strcmp(comando, "edit")==0) {
        } else if(strcmp(comando, "hint")==0) {
        } else if(strcmp(comando, "bk")==0) {
        } else if(strcmp(comando, "undo")==0) {
        } else if(strcmp(comando, "remove")==0) {
        } else if(strcmp(comando, "hard")==0) {
            /* podemos pensar o tempo todo */
            ponder = true;
        } else if(strcmp(comando, "easy")==0) {
            /* pense apenas nossa vez */
            ponder = false;
        } else if(strcmp(comando, "nopost")==0) {
            /* não coloca resultados da busca */
            post = false;
        } else if(strcmp(comando, "post")==0) {
            /* coloca resultados da busca */
            post = true;
        } else if(strcmp(comando, "accepted")==0) {
        } else if(strcmp(comando, "analyze")==0) {
        } else if(strcmp(comando, "name")==0) {
        } else if(strcmp(comando, "rating")==0) {
        } else if(strcmp(comando, "ics")==0) {
        } else if(strcmp(comando, "computer")==0) {
        } else if(strcmp(comando, "pause")==0) {
        } else if(strcmp(comando, "resume")==0) { 
        } else {
            /* jogada do oponete */
            Move m;
            if(strcmp(comando, "usermove") == 0)
                m = parse_move_string(args);
            else
                m = parse_move_string(comando);
            if(not board.validateMove(m)) {
                printf("Illegal move: ");
                print_move(stdout, m);
                printf("\n");
            } else {
                board.move(m);
                play = board.turn;
                fprintf(log_file, "%s\n\n", board.getFen().c_str());
                fprintf(log_file, "rough score %+d\n", board.eval());
            }
        }
        fflush(stdout);
        /* joga de for o caso */
        if(play == board.turn and go) {
            Move m = ComputerPlay(depth, post);
            printf("move %c%c%c%c\n",m.oc+'a',m.ol+'1',m.dc+'a',m.dl+'1');
            fprintf(log_file, "<- move %c%c%c%c\n",m.oc+'a',m.ol+'1',m.dc+'a',m.dl+'1');
            board.move(m);
            fprintf(log_file, "%s\n", board.getFen().c_str());
            fprintf(log_file, "rough score %+d\n\n", board.eval());
        }
        /* verifica fim de jogo */
        if(board.done()) {
            printf("%s\n", board.result.c_str());
            go = 0;
        }

        /* descarga dos buffers */
        fflush(stdout);
        fflush(log_file);
    }
    fprintf(log_file, "saindo...\n");
    return 0;
}

uint32_t hash_hit = 0, hash_miss = 0, exact_hit = 0, false_hit = 0, hash_drops = 0, insert_count = 0, null_moves = 0, null_prunes, pv_fail = 0, node_count = 0;

hash_slot* hash_find() {
    hash_slot* cand = hash_table[board.turn] + board.hash_key % HASH_SIZE;
    if(cand->hash_key != board.hash_key) {
        return 0;
    }
    return cand;
}

void hash_insert(int depth, int score, Move move, uint8_t type) {
    hash_slot* cand = hash_table[board.turn] + (board.hash_key) % HASH_SIZE;

    insert_count ++;

    if(cand->hash_key == board.hash_key) {
        if(cand->depth == depth) {
            if(cand->type > type)
                return;
            if(cand->type == type) {
                if(type == HASH_UPPERBOUND)
                    score = min(score, cand->score);
                if(type == HASH_LOWERBOUND)
                    score = max(score, cand->score);
            }
        }
    } else if(cand->type != HASH_UNUSED) {
        hash_drops ++;
    }

    cand->hash_key = board.hash_key;
    cand->depth = depth;
    cand->score = score;
    cand->type = type;
    cand->move = move;
}

const int cap_table[7]={0,1,2,3,4,5,6};

const int mob_pref_table[7]={0,1,5,4,3,2,0};

Move killer_moves[1024][2];

int dest_table[2][8][8] =
                          {{{  1,  1,  1,  1,  1,  1,  1,  1},
                            {  1,  2,  2,  2,  2,  2,  2,  1},
                            {  1,  2,  3,  3,  3,  3,  2,  1},
                            {  1,  2,  3,  4,  4,  3,  2,  1},
                            {  1,  2,  3,  4,  4,  3,  2,  1},
                            {  1,  2,  3,  3,  3,  3,  2,  1},
                            {  1,  2,  2,  2,  2,  2,  2,  1},
                            {  1,  1,  1,  1,  1,  1,  1,  1}},
                           {{  1,  1,  1,  1,  1,  1,  1,  1},
                            {  1,  2,  2,  2,  2,  2,  2,  1},
                            {  1,  2,  3,  3,  3,  3,  2,  1},
                            {  1,  2,  3,  4,  4,  3,  2,  1},
                            {  1,  2,  3,  4,  4,  3,  2,  1},
                            {  1,  2,  3,  3,  3,  3,  2,  1},
                            {  1,  2,  2,  2,  2,  2,  2,  1},
                            {  1,  1,  1,  1,  1,  1,  1,  1}}};

void init_tables() {
    for(int i=0;i<1024;++i) {
        killer_moves[i][0] = killer_moves[i][1] = Move(0,0,0,0);
    }
}

struct MoveScore {
    Move m;
    int score;
    void setScore() {
        int c = cap_table[board.b[m.dl][m.dc].type];
        int v = cap_table[board.b[m.ol][m.oc].type];
        int o = board.b[m.ol][m.oc].owner;

        score = 8*8*8 * c + 8 * dest_table[o][m.dl][m.dc] + (6 - v);
        if(m == killer_moves[board.move_count][0] or
           m == killer_moves[board.move_count][1]) {
            score += 8*8;
        }
    }
    inline bool operator<(const MoveScore& ms) const {
        return score > ms.score;
    }
};

inline bool compara_move(const Move& m1, const Move& m2) {
    int c1 = cap_table[board.b[m1.dl][m1.dc].type];
    int c2 = cap_table[board.b[m2.dl][m2.dc].type];

    int v1 = cap_table[board.b[m1.ol][m1.oc].type];
    int v2 = cap_table[board.b[m2.ol][m2.oc].type];

    int o = board.b[m1.ol][m1.oc].owner;

    /* capture as pecas de maior valor primeiro */
    if(c1 > c2)
        return true;
    if(c1 < c2)
        return false;

    /* com a peca de menor valor primeiro */
    if(v1 < v2)
        return true;
    if(v1 > v2)
        return false;

    /* jogadas para o centro primeiro */
    if(dest_table[o][m1.dl][m1.dc] > dest_table[o][m2.dl][m2.dc])
        return true;
    return false;
}

void sort_moves(Move* moves, int n) {
    MoveScore ms[512];
    for(int i=0;i<n;++i) {
        ms[i].m = moves[i];
        ms[i].setScore();
    }
    sort(ms, ms+n);
    for(int i=0;i<n;++i) {
        moves[i] = ms[i].m;
    }
    //sort(moves, moves+n, compara_move);
}


#define SEARCH_PV 2
#define SEARCH_ROOT 4
#define INFINITE_SCORE (mate_score + 128)

int search(int depth, int alfa = -INFINITE_SCORE, int beta = INFINITE_SCORE, int flags = SEARCH_ROOT) {
    int n, t;
    uint8_t type;
    MoveInfo mi;
    hash_slot* slot;
    Move list[512];
    Move best_move(0,0,0,0);
    int list_offset = 0;
    int best_score = -INFINITE_SCORE;
    bool is_pv = (flags & SEARCH_PV) != 0;
    bool is_root = (flags & SEARCH_ROOT) != 0;
    bool is_quiescent = (depth <= 0);

    flags &= ~SEARCH_ROOT;

    if(board.repeated()) {
        return 0;
    }

    /* poda os limites de mate */
    /*if(alfa >= mate_score + depth)
        return alfa;
    if(beta <= -(mate_score + depth - 1))
        return beta;*/


    if(not is_quiescent) {
        slot = hash_find();
        
        /* Iterative deepening */
        if(depth >= 3 and slot == 0) {
            search(depth - 2, alfa, beta, SEARCH_ROOT);
            slot = hash_find();
        }

        /* consulta a tabela hash */
        if(slot != 0) {
            if(slot->depth == depth) {
                if(slot->type == HASH_EXACT) {
                    ++ exact_hit;
                    mi = board.move(slot->move);
                    if(not board.repeated()) {
                        board.volta(slot->move, mi);
                        return slot->score;
                    } else {
                        board.volta(slot->move, mi);
                    }
                } else if(slot->type == HASH_UPPERBOUND) {
                    if(slot->score <= alfa) {
                        return slot->score;
                    }
                } else if(slot->type == HASH_LOWERBOUND) {
                    if(slot->score >= beta) {
                        return slot->score;
                    }
                }
            }
            /* pega uma dica da tabela se houver */
            if(slot->move.ol != slot->move.dl or slot->move.oc != slot->move.dc) {
                list[list_offset++] = slot->move;
            }
        }
    }


    /* em quiescent testa apenas as capturas */
    if(is_quiescent) {
        n = board.listAttacks(list + list_offset) + list_offset;
        best_score = max(best_score, board.eval());
    } else {
        n = board.listMoves(list + list_offset, board.turn) + list_offset;
    }


    sort_moves(list+list_offset, n-list_offset);

    ASSERT(n < 512);

    /* null move */
    if(not is_root and not is_pv and depth >= 2 and (depth <= 3 or board.eval() >= beta)) {
        ++ null_moves;
        mi = board.move_null();
        t = -search(depth-3, -beta, -beta+1, flags);
        if(t >= mate_score) t = mate_score - 1;
        board.volta_null(mi);
        if(t >= beta) {
            ++ null_prunes;
            return t;
        }
    }

    for(int i=0;i<n;i++) {
        node_count++;

        /* poda */
        if(best_score >= beta) {
            break;
        }

        /* mate */
        if(board.b[list[i].dl][list[i].dc].type == KING) {
            best_score = (mate_score + depth);
            best_move = list[i];
            break;
        }

        ASSERT(list[i].is_valid());

        /* move */
        mi = board.move(list[i]);

        /* recursão */
        if(depth <= 1) {
            t = -search(depth-1, -beta, -max(alfa, best_score), flags);
        } else { 
            if(i > 0 and not (flags & SEARCH_PV)) {
                t = -search(depth-1, -best_score-1, -best_score, flags);
                if(t > best_score and t < beta) {
                    pv_fail ++;
                    t = -search(depth-1, -beta, -max(alfa, t), flags | SEARCH_PV);
                }
            } else {
                t = -search(depth-1, -beta, -max(alfa, best_score), flags | SEARCH_PV);
            }
        }

        /* volta movimento */
        board.volta(list[i],mi);

        /* atualiza resultado */
        if(t > best_score) {
            best_score = t;
            best_move = list[i];
        }

        /* loga o progresso */
        /*if(is_root) {
            fprintf(log_file, "%d/%d %d/%d ", i, n, best_score, t);
            print_move(log_file, list[i]);
            fprintf(log_file, "\n");
            fflush(log_file);
        }*/

    }

    /* verifica afogamento */
    if(best_score == -(mate_score + depth - 1) and not board.checkCheck()) {
        best_score = 0;
    }

//cut:

    /* registra resultado na tabela hash */
    if(not is_quiescent) {
        if(best_score <= alfa) {
            type = HASH_UPPERBOUND;
        } else if(best_score >= beta) {
            type = HASH_LOWERBOUND;
        } else {
            type = HASH_EXACT;
        }
        hash_insert(depth, best_score, best_move, type);
    }

    if(best_move != Move(0,0,0,0)) {
        ASSERT(board.move_count < 1024);
        if(killer_moves[board.move_count][0] != best_move) {
            killer_moves[board.move_count][1] = killer_moves[board.move_count][0];
            killer_moves[board.move_count][0] = best_move;
        }
    }

    return best_score;
}

Move ComputerPlay(int depth, bool post)
{
    Move best;
    hash_slot* slot;
    MoveInfo mis[128];
    Move hist[128];
    int score = 0;

    hash_hit = 0;
    node_count = 0;
    exact_hit = 0;
    false_hit = 0;
    hash_drops = 0;
    insert_count = 0;
    null_moves = 0;
    null_prunes = 0;
    pv_fail = 0;

    score = search(depth);

    fprintf(log_file, "%s\n", board.getFen().c_str());

    slot = hash_find();

    best = slot->move;

    fprintf(log_file, "\n%+d ", slot->score);
    print_move(log_file, slot->move);
    fprintf(log_file, "\n");

    if(post) {
        printf("%d %d %d %d", depth, score, 0, node_count);
        for(int d=depth;d>=1;--d) {
            search(d, score-1, score);
            slot = hash_find();
            printf(" ");
            print_move(stdout, slot->move);
            mis[d] = board.move(slot->move);
            hist[d] = slot->move;
            score = -score;
            if(d == 1 or board.done()) {
                for(;d<=depth;++d) {
                    board.volta(hist[d], mis[d]);
                }
                break;
            }
        }
        printf("\n");
    }

    fprintf(log_file, "\n");

    fprintf(log_file,"hash hits %d\n", hash_hit);
    fprintf(log_file,"exact hits %d\n", exact_hit);
    fprintf(log_file,"nodes visited %d\n", node_count);
    fprintf(log_file,"null moves %d\n", null_moves);
    fprintf(log_file,"null prunes %d\n", null_prunes);
    fprintf(log_file,"pv fails %d\n", pv_fail);

    /* XXX */
    int hash_usage = 0;
    for(int t=0;t<2;++t) {
        for(uint32_t i=0;i<HASH_SIZE;++i) {
            if(hash_table[t][i].type != HASH_UNUSED) {
                hash_usage ++;
            }
        }
    }

    fprintf(log_file,"hash usage %f\n", double(hash_usage)/double(HASH_SIZE*2)*100.0);
    fprintf(log_file,"hash drops %d\n", hash_drops);
    fprintf(log_file,"false hit %d\n", false_hit);
    fprintf(log_file,"hash inserts %d\n", insert_count);

    fprintf(log_file,"\n");

    fflush(log_file);

    return best;
}

