#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <stdint.h>
#include <cassert>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


using namespace std;

#define CLEAR 0
#define PAWN 1
#define ROOK 2
#define KNIGHT 3
#define BISHOP 4
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

#define dificuldade 6

FILE* log_file = NULL;

struct _move
{
    uint8_t ol,oc,dl,dc;
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

    int8_t pro;
    int8_t castle;
    int8_t passan;
    int8_t captura;
    _pos p;
};

const int valtable[3][7]={{   0,-100,-500,-300,-300,-900,  0},
                          {   0, 100, 500, 300, 300, 900,  0},
                          {   0,   0,   0,   0,   0,   0,  0}};

const int cap_table[7]={0,1,5,3,3,9,10};

const int mate_score = 1000000;

const int dir[2] = {-1,1};

uint64_t hash_code[8][8][2][8];

boost::mt19937 rng;
boost::uniform_int<uint32_t> uint32_interval(0,0xffffffffu);
boost::uniform_int<uint64_t> uint64_interval(0,0xffffffffffffffffllu);

boost::variate_generator<boost::mt19937&, boost::uniform_int<uint32_t> > rand32(rng, uint32_interval);
boost::variate_generator<boost::mt19937&, boost::uniform_int<uint64_t> > rand64(rng, uint64_interval);

void print_move(FILE* file, const _move& m) {
    fprintf(file,"%c%c%c%c", m.oc+'a',m.ol+'1',m.dc+'a',m.dl+'1');
}

struct Board
{
    int8_t passan;
    _pos b[8][8];
    _peca peca[32];
    _coord peca_table[2][8][16];
    int8_t size_table[2][8];
    int8_t npeca;
    int8_t turn;
    int32_t score;
    uint64_t hash_key;

    std::string result;

    Board() {
        clear();
    }

    void clear()
    {
        memset(b, 0, sizeof(b));
        passan = -1;
        npeca = 0;
        memset(size_table, 0, sizeof(size_table));
        turn = WHITE;
        score = 0;
        hash_key = 0;
        for(int l=0;l<8;++l)
            for(int c=0;c<8;++c)
                b[l][c].owner = NONE;
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
        score -= valtable[b[l][c].owner][b[l][c].type];

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
        score += valtable[owner][type];

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
        score += valtable[b[l][c].owner][t] - valtable[b[l][c].owner][b[l][c].type];

        hash_key ^= hash_code[l][c][b[l][c].owner][t];
        hash_key ^= hash_code[l][c][b[l][c].owner][b[l][c].type];

        peca[b[l][c].id].type = t;
        erasePeca2(l, c);
        insertPeca2(l, c, t, b[l][c].owner);
        b[l][c].type = t;
    }

    MoveInfo move(int ol,int oc,int dl,int dc) {
        MoveInfo mi;

        /*if(not checkBoard(__LINE__)) {
            _move m;
            m.ol=ol; m.oc=oc; m.dl=dl; m.dc=dc;
            print_move(log_file, m);
            exit(1);
        }*/

        /* copia peca capturada */
        if(b[dl][dc].type != CLEAR) {
            mi.p = b[dl][dc];
            mi.captura = 1;
            erasePeca(dl, dc);
        } else {
            mi.captura = 0;
        }

        /* movimenta a peca */
        movePeca(ol, oc, dl, dc);

        /* promove */
        if(b[dl][dc].type == PAWN and (dl==7 or dl==0)) {
            setType(dl, dc, QUEEN);
            mi.pro = 1;
        } else {
            mi.pro = 0;
        }

        /* en passan */
        if(b[dl][dc].type == PAWN and mi.captura == 0 and dc != oc) {
            mi.p = b[ol][dc];
            mi.passan = 1;
            erasePeca(ol, dc);
        } else {
            mi.passan = 0;
        }

        /* faz roque */
        mi.castle = 0;
        if(b[dl][dc].type == KING) {
            if((dc-oc)==2) {
                movePeca(dl,7,dl,5);
                mi.castle = 1;
            } else if((dc-oc)==-2) {
                movePeca(dl,0,dl,3);
                mi.castle = 1;
            }
        }

        /* troca a vez */
        turn = OPPONENT(turn);

        //check_hash_key(__LINE__);
        /*if(not checkBoard(__LINE__)) {
            _move m;
            m.ol=ol; m.oc=oc; m.dl=dl; m.dc=dc;
            print_move(log_file, m);
            exit(1);
        }*/

        return mi;
    }

    void volta(int ol,int oc,int dl,int dc, MoveInfo mi) {

        /*if(not checkBoard(__LINE__)) {
            _move m;
            m.ol=ol; m.oc=oc; m.dl=dl; m.dc=dc;
            print_move(log_file, m);
            exit(1);
        }*/

        /* troca a vez */
        turn = OPPONENT(turn);

        /* desfaz roque */
        if(mi.castle) {
            if((dc-oc)==2) {
                movePeca(dl,5,dl,7);
            } else if((dc-oc)==-2) {
                movePeca(dl,3,dl,0);
            }
        }

        /* desfaz promocao */
        if(mi.pro) {
            setType(dl, dc, PAWN);
        }

        /* desfaz passan */
        if(mi.passan) {
            insertPeca(ol, dc, mi.p.type, mi.p.owner);
        }

        /* volta a peca */
        movePeca(dl, dc, ol, oc);

        /* volta peca capturada */
        if(mi.captura) {
            insertPeca(dl, dc, mi.p.type, mi.p.owner);
        }

        //check_hash_key(__LINE__);
        /*if(not checkBoard(__LINE__)) {
            _move m;
            m.ol=ol; m.oc=oc; m.dl=dl; m.dc=dc;
            print_move(log_file, m);
            exit(1);
        }*/

    }


    bool validatePawn(int ol,int oc,int dl,int dc) {
        if(b[ol][oc].owner == WHITE) {
            if(dc == oc) {
                if(dl - ol == 2) {
                    return ol == 1 and b[2][oc].type == CLEAR and b[3][oc].type == CLEAR;
                } else if(dl - ol == 1) {
                    return b[dl][dc].type == CLEAR;
                } else {
                    return false;
                }
            } else if(oc - dc == 1 or oc - dc == -1) {
                return dl - ol == 1 and b[dl][dc].owner == BLACK;
            } else {
                return false;
            }
        } else {
            if(dc == oc) {
                if(dl - ol == -2) {
                    return ol == 6 and b[5][oc].type == CLEAR and b[4][oc].type == CLEAR;
                } else if(dl - ol == -1) {
                    return b[dl][dc].type == CLEAR;
                } else {
                    return false;
                }
            } else if(oc - dc == 1 or oc - dc == -1) {
                return dl - ol == -1 and b[dl][dc].owner == WHITE;
            } else {
                return false;
            }
        }
    }

    bool validateKnight(int ol,int oc,int dl,int dc) {
        int t = (ol-dl)*(oc-dc);
        return (t == 2 or t == -2) and (b[dl][dc].owner != b[ol][oc].owner);
    }

    bool validateBishop(int ol,int oc,int dl,int dc) {
        int di, dj;
        if(abs(oc-dc) != abs(ol-dl))
            return false;
        di = (dc>oc)?1:-1;
        dj = (dl>ol)?1:-1;
        for(int i = oc+di,j=ol+dj;i!=dc;i+=di, j+=dj)
            if(b[j][i].type != CLEAR)
                return false;
        return b[dl][dc].owner != b[ol][oc].owner;
    }

    bool validateKing(int ol,int oc,int dl,int dc) {
        if(abs(oc-dc)==2) {
            return validateCastle(ol, oc, dl, dc);
        }
        if((abs(ol-dl)|abs(oc-dc))!=1)
            return false;
        return b[dl][dc].owner != b[ol][oc].owner;
    }

    bool validateRook(int ol,int oc,int dl,int dc) {
        int di, dj;
        if(!((oc!=dc) ^ (ol!=dl)))
            return false;
        di=dj=0;
        if(dc>oc)
            di=1;
        else if(dc<oc)
            di=-1;
        if(dl>ol)
            dj=1;
        else if(dl<ol)
            dj=-1;
        for(int i=oc+di,j=ol+dj;i!=dc or j!=dl;i+=di, j+=dj)
            if(b[j][i].type)
                return false;
        return b[dl][dc].owner != b[ol][oc].owner;;
    }

    bool validateQueen(int ol,int oc,int dl,int dc) {
        return validateBishop(ol, oc, dl, dc) or
            validateRook(ol, oc, dl, dc);
    }

    bool validateMove(int ol,int oc,int dl,int dc) {
        if(b[ol][oc].owner != turn) {
            return false;
        }
        switch(b[ol][oc].type) {
            case PAWN:
                return validatePawn(ol, oc, dl, dc);
            case KNIGHT:
                return validateKnight(ol, oc, dl, dc);
            case BISHOP:
                return validateBishop(ol, oc, dl, dc);
            case KING:
                return validateKing(ol, oc, dl, dc);
            case ROOK:
                return validateRook(ol, oc, dl, dc);
            case QUEEN:
                return validateQueen(ol, oc, dl, dc);
            default:
                /* Isso não deve acontecer */
                exit(1);
        }
    }

    std::string getFen() {
        const char tabela[3][7]={{'1','p','r','n','b','q','k'},
            {'1','P','R','N','B','Q','K'},
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
        const char tabela[7]={' ','P','T','C','B','D','R'};
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


    bool checkBoard(int line = 0)
    {
        int i,j,n;
        for(i=0,n=0;i<8;i++) {
            for(j=0;j<8;j++) {
                if(b[i][j].type)
                    n++;
            }
        }
        if(n!=npeca) {
            fprintf(log_file,"%d :Contagem nao bate\n", line);
            print(log_file);
            return false;
        }
        for(i=0;i<npeca;i++) {
            if(b[peca[i].l][peca[i].c].type!=peca[i].type or b[peca[i].l][peca[i].c].owner!=peca[i].owner or b[peca[i].l][peca[i].c].id!=i) {
                fprintf(log_file,"%d: (%d,%d) -> peca(type=%d,owner=%d,id=%d) Board(type=%d,owner=%d,id=%d)\n",line,peca[i].l,peca[i].c,peca[i].type,peca[i].owner,i,b[peca[i].l][peca[i].c].type,b[peca[i].l][peca[i].c].owner,b[peca[i].l][peca[i].c].id);
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
                        fprintf(log_file, "%d: peca_table nao bate\n", line);
                        fprintf(log_file,"%d: peca_table[%d][%d][%d] = (l=%d,c=%d) b(type=%d,owner=%d,id2=%d)\n",line,t, p, i, l, c, b[l][c].type,b[l][c].owner,b[l][c].id2);
                        print(log_file);
                        return false;
                    }
                }
            }
        }

        for(int l=0;l<8;++l) {
            for(int c=0;c<8;++c) {
                if(b[l][c].type == CLEAR)
                    continue;
                int o = (b[l][c].owner+1)/2;
                if(peca_table[o][b[l][c].type][b[l][c].id2].l != l or
                        peca_table[o][b[l][c].type][b[l][c].id2].c != c) {
                    fprintf(log_file, "%d: peca_table nao bate\n", line);
                    fprintf(log_file,"%d: peca_table[%d][%d][%d] = (l=%d,c=%d) b = (type=%d,owner=%d,id2=%d)\n",line,o,b[l][c].type,b[l][c].id2, l, c, b[l][c].type,b[l][c].owner,b[l][c].id2);
                    print(log_file);
                    return false;
                }
                if(peca[b[l][c].id].type != b[l][c].type or
                        peca[b[l][c].id].l != l or
                        peca[b[l][c].id].c != c or
                        peca[b[l][c].id].owner != b[l][c].owner) {
                    fprintf(log_file,"%d: peca[%d] = (type=%d,owner=%d) b[%d][%d] = (type=%d,owner=%d,id=%d)\n",line,b[l][c].id, peca[b[l][c].id].type,peca[b[l][c].type].owner, l, c, b[l][c].type, b[l][c].owner, b[l][c].id);
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
            if(validatePawn(peca_table[player][PAWN][i].l, peca_table[player][PAWN][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[player][KNIGHT];++i) {
            if(validateKnight(peca_table[player][KNIGHT][i].l, peca_table[player][KNIGHT][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[player][BISHOP];++i) {
            if(validateBishop(peca_table[player][BISHOP][i].l, peca_table[player][BISHOP][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[player][ROOK];++i) {
            if(validateRook(peca_table[player][ROOK][i].l, peca_table[player][ROOK][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[player][QUEEN];++i) {
            if(validateQueen(peca_table[player][QUEEN][i].l, peca_table[player][QUEEN][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[player][KING];++i) {
            if(validateKing(peca_table[player][KING][i].l, peca_table[player][KING][i].c, l, c))
                return true;
        }
        return false;
    }

    bool validateCastle(int ol, int oc, int dl, int dc)
    {
        int torre;
        if(dc-oc == 2)
            torre = 7;
        else if(dc-oc == -2)
            torre = 1;
        else
            return false;
        if((b[ol][oc].owner == WHITE and oc != 0) or (b[ol][oc].owner == BLACK and oc != 7))
            return false;
        if(b[ol][torre].type != ROOK or b[ol][oc].owner != b[ol][torre].owner)
            return false;
        if(b[dl][dc].type != CLEAR or b[dl][(dc+oc)/2].type != CLEAR)
            return false;
        int op = OPPONENT(b[ol][oc].owner);
        if(checkAttack(dl,dc,op) or checkAttack(dl,(dc+oc)/2,op) or checkAttack(ol,oc,op)) {
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

    int listWhitePawnMoves(int l, int c, _move *list) {
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

        return n;
    }

    int listBlackPawnMoves(int l, int c, _move *list) {
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

        return n;
    }

    int listKnightMoves(int l, int c, _move *list) {
        static const int dl[] = {2,1,-1,-2,-2,-1,1,2};
        static const int dc[] = {1,2,2,1,-1,-2,-2,-1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner != turn) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listRookMoves(int l, int c, _move *list) {
        static const int dl[] = { 1, 0,-1, 0};
        static const int dc[] = { 0, 1, 0,-1};
        int n = 0;
        int l1, c1;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]) {
                put(l1,c1);
            }
            if(VALID_POS(l1,c1) and b[l1][c1].owner != turn) {
                put(l1,c1);
            }
        }
        return n;
    }

    int listBishopMoves(int l, int c, _move *list) {
        static const int dl[] = {1, 1,-1,-1};
        static const int dc[] = {1,-1,-1, 1};
        int l1,c1;
        int n = 0;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]) {
                put(l1,c1);
            }
            if(VALID_POS(l1,c1) and b[l1][c1].owner != turn) {
                put(l1,c1);
            }
        }
        return n;
    }

    int listKingMoves(int l, int c, _move *list) {
        static const int dl[] = { 1, 1, 1, 0,-1,-1,-1, 0};
        static const int dc[] = { 1, 0,-1,-1,-1, 0, 1, 1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner != turn) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listQueenMoves(int l, int c, _move *list) {
        int n = 0;

        n += listRookMoves(l, c, list);
        n += listBishopMoves(l, c, list + n);

        return n;
    }

    int listMoves(_move *list)
    {
        int n = 0;

        if(turn == WHITE) {
            for(int i=0;i<size_table[WHITE][PAWN];++i) {
                n += listWhitePawnMoves(peca_table[WHITE][PAWN][i].l, peca_table[WHITE][PAWN][i].c, list + n);
            }
        } else {
            for(int i=0;i<size_table[BLACK][PAWN];++i) {
                n += listBlackPawnMoves(peca_table[BLACK][PAWN][i].l, peca_table[BLACK][PAWN][i].c, list + n);
            }
        }

        for(int i=0;i<size_table[turn][KNIGHT];++i) {
            n += listKnightMoves(peca_table[turn][KNIGHT][i].l, peca_table[turn][KNIGHT][i].c, list + n);
        }

        for(int i=0;i<size_table[turn][BISHOP];++i) {
            n += listBishopMoves(peca_table[turn][BISHOP][i].l, peca_table[turn][BISHOP][i].c, list + n);
        }

        for(int i=0;i<size_table[turn][ROOK];++i) {
            n += listRookMoves(peca_table[turn][ROOK][i].l, peca_table[turn][ROOK][i].c, list + n);
        }

        for(int i=0;i<size_table[turn][QUEEN];++i) {
            n += listQueenMoves(peca_table[turn][QUEEN][i].l, peca_table[turn][QUEEN][i].c, list + n);
        }

        for(int i=0;i<size_table[turn][KING];++i) {
            n += listKingMoves(peca_table[turn][KING][i].l, peca_table[turn][KING][i].c, list + n);
        }

        return n;
    }

    int listWhitePawnAttacks(int l, int c, int op, _move* list) {
        int n = 0;
        if(c>0 and b[l+1][c-1].owner == op)
            put(l+1,c-1);
        if(c<7 and b[l+1][c+1].owner == op)
            put(l+1,c+1);
        return n;
    }

    int listBlackPawnAttacks(int l, int c, int op, _move* list) {
        int n = 0;
        if(c>0 and b[l-1][c-1].owner == op)
            put(l-1,c-1);
        if(c<7 and b[l-1][c+1].owner == op)
            put(l-1,c+1);
        return n;
    }


    int listKnightAttacks(int l, int c, int op, _move* list) {
        static const int dl[] = {2,1,-1,-2,-2,-1,1,2};
        static const int dc[] = {1,2,2,1,-1,-2,-2,-1};
        int n = 0;
        for(int i=0;i<8;++i) {
            if(VALID_POS(l+dl[i],c+dc[i]) and b[l+dl[i]][c+dc[i]].owner == op) {
                put(l+dl[i],c+dc[i]);
            }
        }
        return n;
    }

    int listBishopAttacks(int l, int c, int op, _move* list) {
        static const int dl[] = {1, 1,-1,-1};
        static const int dc[] = {1,-1,-1, 1};
        int l1,c1;
        int n = 0;
        for(int i=0;i<4;++i) {
            for(l1=l+dl[i],c1=c+dc[i];VALID_POS(l1,c1) and b[l1][c1].type == CLEAR;l1+=dl[i],c1+=dc[i]);
            if(VALID_POS(l1,c1) and b[l1][c1].owner == op)
                put(l1,c1);
        }
        return n;
    }

    int listRookAttacks(int l, int c, int op, _move* list) {
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

    int listQueenAttacks(int l, int c, int op, _move* list) {
        int n = 0;
        n += listBishopAttacks(l, c, op, list);
        n += listRookAttacks(l, c, op, list + n);
        return n;
    }

    int listKingAttacks(int l, int c, int op, _move* list) {
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

    int listAttacks(_move* list) {

        int n = 0;

        if(turn == WHITE) {
            for(int i=0;i<size_table[WHITE][PAWN];++i) {
                n += listWhitePawnAttacks(peca_table[WHITE][PAWN][i].l, peca_table[WHITE][PAWN][i].c, OPPONENT(turn), list + n);
            }
        } else {
            for(int i=0;i<size_table[BLACK][PAWN];++i) {
                n += listBlackPawnAttacks(peca_table[BLACK][PAWN][i].l, peca_table[BLACK][PAWN][i].c, OPPONENT(turn), list + n);
            }
        }

        for(int i=0;i<size_table[turn][KNIGHT];++i) {
            n += listKnightAttacks(peca_table[turn][KNIGHT][i].l, peca_table[turn][KNIGHT][i].c, OPPONENT(turn), list + n);
        }

        for(int i=0;i<size_table[turn][BISHOP];++i) {
            n += listBishopAttacks(peca_table[turn][BISHOP][i].l, peca_table[turn][BISHOP][i].c, OPPONENT(turn), list + n);
        }

        for(int i=0;i<size_table[turn][ROOK];++i) {
            n += listRookAttacks(peca_table[turn][ROOK][i].l, peca_table[turn][ROOK][i].c, OPPONENT(turn), list + n);
        }

        for(int i=0;i<size_table[turn][QUEEN];++i) {
            n += listQueenAttacks(peca_table[turn][QUEEN][i].l, peca_table[turn][QUEEN][i].c, OPPONENT(turn), list + n);
        }

        for(int i=0;i<size_table[turn][KING];++i) {
            n += listKingAttacks(peca_table[turn][KING][i].l, peca_table[turn][KING][i].c, OPPONENT(turn), list + n);
        }

        return n;
    }

    bool done() {
        _move moves[1024];
        MoveInfo mi;
        int kl = 0, kc = 0;
        int n = listMoves(moves);
        bool res = true;

        kl = peca_table[turn][KING][0].l;
        kc = peca_table[turn][KING][0].c;

        for(int i=0;i<n;++i) {
            mi = move(moves[i].ol, moves[i].oc, moves[i].dl, moves[i].dc);
            if(kl == moves[i].ol and kc == moves[i].oc) {
                if(not checkAttack(moves[i].dl, moves[i].dc, turn)) {
                    res = false;
                }
            } else {
                if(not checkAttack(kl, kc, turn)) {
                    res = false;
                }
            }
            volta(moves[i].ol, moves[i].oc, moves[i].dl, moves[i].dc, mi);
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

    int eval() {
        if(turn == WHITE) {
            return this->score;
        } else {
            return - this->score;
        }
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
} board;


struct board_code {
    uint32_t code[8];

    void encode() {
        for(int l=0;l<8;++l) {
            code[l] = 0;
            for(int c=0;c<8;++c) {
                code[l] = (code[l]<<4) | (board.b[l][c].owner + (board.b[l][c].type) * 3);
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

_move ComputerPlay(int depth);

#define HASH_UNUSED 0
#define HASH_LOWERBOUND 1
#define HASH_UPPERBOUND 2
#define HASH_EXACT 2

struct hash_slot {
    uint64_t hash_key;
    int32_t score;
    uint8_t depth;
    uint8_t type;
    _move move;
    board_code code;
};

static const unsigned long prime_list[] = {
    53ul,         97ul,         193ul,        389ul,       769ul,
    1543ul,       3079ul,       6151ul,       12289ul,     24593ul,
    49157ul,      98317ul,      196613ul,     393241ul,    786433ul,
    1572869ul,    3145739ul,    6291469ul,    12582917ul,  25165843ul,
    50331653ul,   100663319ul,  201326611ul,  402653189ul, 805306457ul,
    1610612741ul, 3221225473ul, 4294967291ul
};

const uint32_t HASH_SIZE = 6291469;
hash_slot hash_table[2][HASH_SIZE];

void init() {
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
            hash_table[turn][i].type = HASH_UNUSED;
        }
    }
}


_move parse_move_string(std::string move_str) {
    _move resp;
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

_move list[256];

long long node_count;
int random_play;

int main(int argc, char** argv)
{
    char comando[1024];
    char* args;
    int play = BLACK;
    int go = 0;
    random_play = 0;

    if(argc==1)
        log_file = fopen("tmp.txt","w");
    else
        log_file = fopen(argv[1],"w");

    int seed = time(NULL);
    fprintf(log_file,"seed %d\n", seed);

    init();
    srand48(seed);
    board.setInitial();
    board.checkBoard(__LINE__);
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
        } else if(strcmp(comando, "protover")==0) {
            printf("feature myname=\"raphael-chess%d\" ping=1 usermove=1 draw=0 variants=\"normal\" sigint=0 sigterm=0 done=1\n",dificuldade);
        } else if(strcmp(comando, "new")==0) {
            board.setInitial();
            play = BLACK;
            go = 1;
        } else if(strcmp(comando, "variant")==0) {
        } else if(strcmp(comando, "quit")==0) {
            break;
        } else if(strcmp(comando, "random")==0) {
            random_play = not random_play;
        } else if(strcmp(comando, "force")==0) {
            go = 0;
        } else if(strcmp(comando, "go")==0) {
            if(go == 0) {
                play = board.turn;
                go = 1;
            }
        } else if(strcmp(comando, "playother")==0) {
            play = board.turn^1;
            go=1;
        } else if(strcmp(comando, "white")==0) {
        } else if(strcmp(comando, "black")==0) {
        } else if(strcmp(comando, "level")==0) {
        } else if(strcmp(comando, "st")==0) {
        } else if(strcmp(comando, "sd")==0) {
        } else if(strcmp(comando, "time")==0) {
        } else if(strcmp(comando, "otim")==0) {
        } else if(strcmp(comando, "board")==0) {
            board.print(stdout);
        } else if(strcmp(comando, "ping")==0) {
            printf("pong %s\n", args);
        } else if(strcmp(comando, "usermove")==0) {
            /* play it*/
            _move m = parse_move_string(args);
            board.move(m.ol, m.oc, m.dl, m.dc);
            play = board.turn;
            fprintf(log_file, "%s\n\n", board.getFen().c_str());
        } else if(strcmp(comando, "draw")==0) {
        } else if(strcmp(comando, "result")==0) {
            go=0;
        } else if(strcmp(comando, "setboard")==0) {
            board.setFen(args);
        } else if(strcmp(comando, "edit")==0) {
        } else if(strcmp(comando, "hint")==0) {
        } else if(strcmp(comando, "bk")==0) {
        } else if(strcmp(comando, "undo")==0) {
        } else if(strcmp(comando, "remove")==0) {
        } else if(strcmp(comando, "hard")==0) {
        } else if(strcmp(comando, "easy")==0) {
        } else if(strcmp(comando, "nopost")==0) {
        } else if(strcmp(comando, "post")==0) {
        } else if(strcmp(comando, "accepted")==0) {
        } else if(strcmp(comando, "analyze")==0) {
        } else if(strcmp(comando, "name")==0) {
        } else if(strcmp(comando, "rating")==0) {
        } else if(strcmp(comando, "ics")==0) {
        } else if(strcmp(comando, "computer")==0) {
        } else if(strcmp(comando, "pause")==0) {
        } else if(strcmp(comando, "resume")==0) { 
        } else {
            /* play it*/
            _move m = parse_move_string(comando);
            board.move(m.ol, m.oc, m.dl, m.dc);
            play = board.turn;
            fprintf(log_file, "%s\n\n", board.getFen().c_str());
        }
        fflush(stdout);
        if(play==board.turn and go) {
            _move m = ComputerPlay(dificuldade);
            printf("move %c%c%c%c\n",m.oc+'a',m.ol+'1',m.dc+'a',m.dl+'1');
            fprintf(log_file, "<- move %c%c%c%c\n",m.oc+'a',m.ol+'1',m.dc+'a',m.dl+'1');
            board.move(m.ol, m.oc, m.dl, m.dc);
            fprintf(log_file, "%s\n\n", board.getFen().c_str());
        }
        if(board.done()) {
            printf("%s\n", board.result.c_str());
            go = 0;
        }
        fflush(stdout);
        fflush(log_file);
    }
    fprintf(log_file, "saindo...\n");
    return 0;
}

const int code_table[3][7] = 
{{12,0 ,1 ,2 ,3 ,4 ,5},
 {12,6 ,7 ,8 ,9 ,10,11},
 {12,12,12,12,12,12,12}};

uint64_t hash_hit = 0, hash_miss = 0, exact_hit = 0, false_hit = 0, hash_drops;

hash_slot* hash_find(int depth) {
    hash_slot* cand = hash_table[board.turn] + board.hash_key % HASH_SIZE;
    if(cand->type == HASH_UNUSED or cand->hash_key != board.hash_key or cand->depth < depth) {
        return 0;
    }
    if(enc() != cand->code) {
        false_hit ++;
    }
    hash_hit ++;
    return cand;
}

void hash_insert(int depth, int score, _move move, uint8_t type) {
    hash_slot* cand = hash_table[board.turn] + (board.hash_key) % HASH_SIZE;
    if(cand->type != HASH_UNUSED or cand->hash_key != board.hash_key or depth > cand->depth or (depth == cand->depth and type == HASH_EXACT)) {
        /*if(cand->type != HASH_UNUSED) {
            hash_drops ++;
        }*/
        cand->hash_key = board.hash_key;
        cand->depth = depth;
        cand->score = score;
        cand->type = type;
        cand->move = move;
    } else {
        if(enc() != cand->code) {
            false_hit ++;
        }
        /* TODO update a slot */
        /*if(depth == cand->depth) {
            if(board.turn == WHITE and score > cand->score) {
                cand->best = best;
                cand->score = score;
            } 
            if(board.turn == BLACK and score < cand->score) {
                cand->best = best;
                cand->score = cand->score;
            }
        }*/
    }
    cand->code.encode();
}

int score_move(const _move& m) {
    if((m.dl==0 or m.dl==7) and board.b[m.ol][m.oc].type==PAWN) {
        return cap_table[QUEEN] - cap_table[PAWN];
    } else {
        return cap_table[board.b[m.dl][m.dc].type];
    }
}

bool compara_move(const _move& m1, const _move& m2) {
    return score_move(m1) > score_move(m2);
}

_move ordena_table[11][256];
int ordena_count[11];
void ordena_move(_move* begin, _move* end) {
    for(int i=0;i<11;++i) ordena_count[i]=0;
    for(_move* p1 = begin; p1 != end; ++p1) {
        int s = score_move(*p1);
        ordena_table[s][ordena_count[s]++] = *p1;
    }
    for(int i=10;i>=0;--i) {
        for(int j=0;j<ordena_count[i];++j)
            *(begin++) = ordena_table[i][j];
    }
}

#define SEARCH_QUIESCENT 1
#define DISABLE_HASH 2
#define INFINITE_SCORE (mate_score + 128)

int search(int depth, int alfa = -INFINITE_SCORE, int beta = INFINITE_SCORE, int flags = 0) {
    int n, t;
    uint8_t type;
    MoveInfo mi;
    hash_slot* slot;
    _move list[512];
    _move best_move;
    int best_score;

    best_score = -INFINITE_SCORE;
    best_move.ol = best_move.oc = best_move.dl = best_move.dc = 0;

    /* consulta a tabela hash */
    if(not (flags & DISABLE_HASH) and (slot = hash_find(depth)) != 0) {
        if(slot->type == HASH_EXACT) {
            ++ exact_hit;
            t = search(depth, -INFINITE_SCORE, INFINITE_SCORE, flags | DISABLE_HASH);
            if(slot->score != t) {
                printf("merda! 1\n");
            }
            return slot->score;
        } /*else if(slot->type == HASH_UPPERBOUND) {
            beta = min(beta, slot->score);
        } else {
            alfa = max(alfa, slot->score);
        }
        if(alfa >= beta) {
            return slot->score;
        }*/
    }

    /* em quiescent testa apenas as capturas */
    if(flags & SEARCH_QUIESCENT) {
        n = board.listAttacks(list);
        best_score = max(best_score, board.eval());
    } else {
        n = board.listMoves(list);
    }
    ordena_move(list, list+n);

    for(int i=0;i<n;i++) {
        node_count++;

        /* loga o progresso */
        /*if(depth == dificuldade) {
            fprintf(log_file, "%d/%d\n", i, n);
            fflush(log_file);
        }*/

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

        /* move */
        mi = board.move(list[i].ol,list[i].oc,list[i].dl,list[i].dc);

        /* recursão */
        if(depth == 1) {
            if(mi.p.type == CLEAR) {
                t = board.eval();
            } else {
                /* extende a busca em caso de captura na folha */
                t = -search(1, -beta, -max(alfa, best_score), flags | SEARCH_QUIESCENT);
            }
        } else {
            t = -search(depth-1, -beta, -max(alfa, best_score), flags);
        }

        /* volta movimento */
        board.volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc,mi);

        /* atualiza resultado */
        if(best_score < t) {
            best_score = t;
            best_move = list[i];
        }
    }

    /* registra resultado na tabela hash */
    if(not (flags & DISABLE_HASH) and not (flags & SEARCH_QUIESCENT)) {
        if(best_score <= alfa) {
            type = HASH_UPPERBOUND;
        } else if(best_score >= beta) {
            type = HASH_LOWERBOUND;
        } else {
            type = HASH_EXACT;
            t = search(depth, -INFINITE_SCORE, INFINITE_SCORE, flags | DISABLE_HASH);
            if(t != best_score) {
                printf("merda 2\n");
            }
        }
        //if(best_score <= -mate_score)
        //    type = HASH_EXACT;
        hash_insert(depth, best_score, best_move, type);
    }

    return best_score;
}

_move ComputerPlay(int depth)
{
    int min = -0x7fffffff, max = 0x7fffffff;
    _move best;
    hash_slot* slot;
    MoveInfo mis[128];
    _move hist[128];

    hash_hit = 0;
    node_count = 0;
    exact_hit = 0;
    false_hit = 0;
    hash_drops = 0;

    search(depth, min, max);

    slot = hash_find(depth);

    best = slot->move;

    fprintf(log_file, "\n%+d ", slot->score);
    print_move(log_file, slot->move);

    if(0) {
        for(int i=0;i<depth;++i) {
            mis[i] = board.move(slot->move.ol,slot->move.oc,slot->move.dl,slot->move.dc);
            hist[i] = slot->move;
            slot = hash_find(depth - i - 1);
            if(i == depth - 1 or slot == 0) {
                for(;i>=0;--i) {
                    board.volta(hist[i].ol, hist[i].oc, hist[i].dl, hist[i].dc, mis[i]);
                }
                break;
            }
            fprintf(log_file, " ");
            print_move(log_file, slot->move);
        }
    } else if(1) {
        for(int i=0;i<depth;++i) {
            mis[i] = board.move(slot->move.ol,slot->move.oc,slot->move.dl,slot->move.dc);
            hist[i] = slot->move;
            if(i == depth - 1 and not board.done()) {
                for(;i>=0;--i) {
                    board.volta(hist[i].ol, hist[i].oc, hist[i].dl, hist[i].dc, mis[i]);
                }
                break;
            }
            search(depth - i - 1, min, max);
            slot = hash_find(depth - i - 1);
            fprintf(log_file, " %+d ", slot->score);
            print_move(log_file, slot->move);
        }
    }

    fprintf(log_file, "\n");

    fprintf(log_file,"hash hits %lld\n", hash_hit);
    fprintf(log_file,"exact hits %lld\n", exact_hit);
    fprintf(log_file,"nodes visited %lld\n", node_count);

    int hash_usage = 0;
    for(int t=0;t<2;++t) {
        for(uint32_t i=0;i<HASH_SIZE;++i) {
            if(hash_table[t][i].type != HASH_UNUSED) {
                hash_usage ++;
            }
        }
    }

    fprintf(log_file,"hash usage %f\n", double(hash_usage)/double(HASH_SIZE*2)*100.0);
    fprintf(log_file,"hash drops %lld\n", hash_drops);
    fprintf(log_file,"false hit %lld\n", false_hit);

    fprintf(log_file,"\n");

    fflush(log_file);

    return best;
}

