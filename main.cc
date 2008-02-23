#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

using namespace std;

#define CLEAR 0
#define PAWN 1
#define ROOK 2
#define KNIGHT 3
#define BISHOP 4
#define QUEEN 5
#define KING 6
#define WHITE 1  
#define BLACK -1 
#define CASTLE 2
#define vl(x) ((x)<0 && (x)>7)
#define put() {list[n].ol=l;list[n].oc=c;list[n].dl=y;list[n].dc=x;n++;}

FILE* log_file = NULL;

struct _move
{
    int ol,oc,dl,dc;
};

struct _peca
{
    int l,c;
    int type;
    int owner;
};

struct _coord {
    int l, c;
};

struct _pos
{
    int id;
    int id2;
    int type;
    int owner;
};

struct MoveInfo {
    int pro;
    int castle;
    _pos p;
};

struct Board
{
    int passan;
    _pos b[8][8];
    _peca peca[32];
    _coord peca_table[2][8][16];
    int size_table[2][8];
    int npeca;
    int turn;

    Board() {
        clear();
    }

    void clear()
    {
        memset(b, 0, sizeof(b));
        passan = -1;
        npeca = 0;
        memset(size_table, 0, sizeof(size_table));
        turn = 1;
    }

    void setTables()
    {
        npeca=0;
        memset(size_table, 0, sizeof(size_table));
        for(int l=0;l<8;l++) {
            for(int c=0;c<8;c++) {
                if(b[l][c].type == 0)
                    continue;
                insertPeca(l, c, b[l][c].type, b[l][c].owner);
            }
        }
    }

    void setFen(const string& fen)
    {
        int linha=7, coluna=0;

        this->clear();

        for(int i=0;fen[i]!=' ';++i) {
            if(fen[i]=='/') {
                --linha, coluna=0;
            } else if(isdigit(fen[i])) {
                coluna += fen[i]-'0';
            } else {
                b[linha][coluna].owner = isupper(fen[i])?WHITE:BLACK;
                switch(tolower(fen[i])) {
                    case 'r': b[linha][coluna].type = ROOK; break;
                    case 'n': b[linha][coluna].type = KNIGHT; break;
                    case 'b': b[linha][coluna].type = BISHOP; break;
                    case 'q': b[linha][coluna].type = QUEEN; break;
                    case 'k': b[linha][coluna].type = KING; break;
                    case 'p': b[linha][coluna].type = PAWN; break;
                }
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
        this->setTables();
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
        p.owner = (p.owner+1)/2;
        int tmp = size_table[p.owner][p.type]--;
        if(p.id2 != tmp - 1) {
            peca_table[p.owner][p.type][p.id2] = peca_table[p.owner][p.type][tmp-1];
            b[peca_table[p.owner][p.type][p.id2].l][peca_table[p.owner][p.type][p.id2].c].id2 = p.id2;
        }
    }

    void erasePeca(int l, int c) {
        erasePeca1(l, c);
        erasePeca2(l, c);
        b[l][c].type = CLEAR;
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
        owner = (owner+1)/2;
        int tmp = size_table[owner][type]++;
        peca_table[owner][type][tmp].l = l;
        peca_table[owner][type][tmp].c = c;
        b[l][c].id2 = tmp;
    }

    void insertPeca(int l, int c, int type, int owner) {
        insertPeca1(l, c, type, owner);
        insertPeca2(l, c, type, owner);
        b[l][c].type = type;
        b[l][c].owner = owner;
    }

    void movePeca(int ol,int oc,int dl,int dc) {
        _pos s;
        s = b[ol][oc];
        peca[s.id].l=dl;
        peca[s.id].c=dc;
        peca_table[(s.owner+1)/2][s.type][s.id2].l=dl;
        peca_table[(s.owner+1)/2][s.type][s.id2].c=dc;
        b[dl][dc] = b[ol][oc];
        b[ol][oc].type = CLEAR;
    }

    void setType(int l, int c, int t) {
        b[l][c].type = t;
        peca[b[l][c].id].type=0;
        erasePeca2(l, c);
        insertPeca2(l, c, t, b[l][c].owner);
    }

    MoveInfo move(int ol,int oc,int dl,int dc) {
        MoveInfo mi;

        //checkBoard(__LINE__);
        mi.p = b[dl][dc];
        if(mi.p.type != CLEAR) {
            erasePeca(dl, dc);
        }

        movePeca(ol, oc, dl, dc);

        if(b[dl][dc].type == PAWN && (dl==7 || dl==0)) {
            setType(dl, dc, QUEEN);
            mi.pro = 1;
        } else {
            mi.pro = 0;
        }

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

        turn = -turn;
        //checkBoard(__LINE__);

        return mi;
    }

    void volta(int ol,int oc,int dl,int dc, MoveInfo mi) {
        turn = -turn;

        //checkBoard(__LINE__);
        if(mi.castle) {
            if((dc-oc)==2) {
                movePeca(dl,5,dl,7);
            } else if((dc-oc)==-2) {
                movePeca(dl,3,dl,0);
            }
        }

        if(mi.pro) {
            setType(dl, dc, PAWN);
        }

        movePeca(dl, dc, ol, oc);

        if(mi.p.type != CLEAR) {
            insertPeca(dl, dc, mi.p.type, mi.p.owner);
        }
        //checkBoard(__LINE__);
    }


    bool validatePawn(int ol,int oc,int dl,int dc) {
        if(b[dl][dc].type == CLEAR) {
            if(dc!=oc)
                return false;
            if((dl-ol)*turn>2 || (dl-ol)*turn<=0)
                return false;
            if(ol!=((7-turn*5)/2) && (dl-ol)*turn==2)
                return false;
            if(ol==(7+turn*5)/2 && (dl-ol)*turn==2 && b[ol+dl/2][oc].type != CLEAR)
                return false;
            return true;
        } else if((oc-dc)==1 || (oc-dc)==-1) {
            /* FIXME */
            return abs(dl-ol)==1;
        } else {
            return false;
        }
    }

    bool validateKnight(int ol,int oc,int dl,int dc) {
        return abs((ol-dl)*(oc-dc))==2 &&( (b[dl][dc].owner)==-turn || b[dl][dc].type == CLEAR);
    }

    bool validateBishop(int ol,int oc,int dl,int dc) {
        int di, dj;
        if(abs(oc-dc)!=abs(ol-dl))
            return false;
        if(dc>oc)
            di=1;
        else
            di=-1;
        if(dl>ol)
            dj=1;
        else
            dj=-1;
        for(int i=oc+di,j=ol+dj;i!=dc;i+=di, j+=dj)
            if(b[j][i].type != CLEAR)
                return false;
        if(b[dl][dc].owner == turn && b[dl][dc].type != CLEAR)
            return false;
        return true;
    }

    bool validateKing(int ol,int oc,int dl,int dc) {
        int i;
        if(abs(oc-dc)==2 && dl==ol) {
            if(dc>oc)
                i=1;
            else
                i=-1;
            return validateCastle(i);
        }
        if((abs(ol-dl)|abs(oc-dc))!=1)
            return false;
        if(b[dl][dc].owner == turn && b[dl][dc].type != CLEAR)
            return false;
        return true;
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
        for(int i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
            if(b[j][i].type)
                return false;
        if(b[dl][dc].type != CLEAR && b[dl][dc].owner == turn)
            return false;
        return true;
    }

    bool validateQueen(int ol,int oc,int dl,int dc) {
        return validateBishop(ol, oc, dl, dc) or
            validateRook(ol, oc, dl, dc);
    }

    bool validateMove(int ol,int oc,int dl,int dc) {
        if(b[ol][oc].type == CLEAR || (b[ol][oc].owner) != turn) {
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


    void print(FILE * f)
    {
        const char tabela[7]={' ','P','T','C','B','D','R'};
        const char tabela2[3]={'P',' ','B'};

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
                linha[3*i+2]=tabela2[b[l][i].owner+1];
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
    }


    void checkBoard(int line = 0)
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
            exit(1);
            return;
        }
        for(i=0;i<npeca;i++) {
            if(b[peca[i].l][peca[i].c].type!=peca[i].type || b[peca[i].l][peca[i].c].owner!=peca[i].owner || b[peca[i].l][peca[i].c].id!=i) {
                fprintf(log_file,"%d: (%d,%d) -> peca(type=%d,owner=%d,id=%d) Board(type=%d,owner=%d,id=%d)\n",line,peca[i].l,peca[i].c,peca[i].type,peca[i].owner,i,b[peca[i].l][peca[i].c].type,b[peca[i].l][peca[i].c].owner,b[peca[i].l][peca[i].c].id);
                print(log_file);
                exit(1);
                return;
            }
        }
        for(int t=0;t<2;++t) {
            for(int p=1;p<7;++p) {
                for(int i=0;i<size_table[t][p]; ++i) {
                    if(b[peca_table[t][p][i].l][peca_table[t][p][i].c].owner != (t*2-1) or
                            b[peca_table[t][p][i].l][peca_table[t][p][i].c].type != p or
                            b[peca_table[t][p][i].l][peca_table[t][p][i].c].id2 != i) {
                        int l = peca_table[t][p][i].l;
                        int c = peca_table[t][p][i].c;
                        fprintf(log_file,"%d: peca_table[%d][%d][%d] = (l=%d,c=%d) b(type=%d,owner=%d,id2=%d)\n",line,t, p, i, l, c, b[l][c].type,b[l][c].owner,b[l][c].id2);
                        fprintf(log_file, "%d: peca_table nao bate\n", line);
                        print(log_file);
                        exit(1);
                        return;
                    }
                }
            }
        }
    }

    bool checkAttack(int l,int c)
    {
        int t = (turn+1)/2;
        for(int i=0;i<size_table[t][PAWN];++i) {
            if(validatePawn(peca_table[t][PAWN][i].l, peca_table[t][PAWN][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[t][KNIGHT];++i) {
            if(validateKnight(peca_table[t][KNIGHT][i].l, peca_table[t][KNIGHT][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[t][BISHOP];++i) {
            if(validateBishop(peca_table[t][BISHOP][i].l, peca_table[t][BISHOP][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[t][ROOK];++i) {
            if(validateRook(peca_table[t][ROOK][i].l, peca_table[t][ROOK][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[t][QUEEN];++i) {
            if(validateQueen(peca_table[t][QUEEN][i].l, peca_table[t][QUEEN][i].c, l, c))
                return true;
        }
        for(int i=0;i<size_table[t][KING];++i) {
            if(validateKing(peca_table[t][KING][i].l, peca_table[t][KING][i].c, l, c))
                return true;
        }
        return false;
    }

    bool validateCastle(int dir)
    {
        int i,kl,kc;
        kl=(7-turn*7)/2;
        kc=4;
        if(b[kl][kc].type!=KING || b[kl][kc].owner!=turn)
            return false;
        if(b[kl][(7+7*dir)/2].type != ROOK || b[kl][(7+7*dir)/2].owner!=turn)
            return false;
        for(i=kc+dir;i>0 && i<7;i+=dir)
            if(b[kl][i].type)
                return false;
        /* vo fingir que ta vez do oponente */
        turn = -turn;
        if(checkAttack(kl,kc)) {
            turn = -turn;
            return false;
        }
        for(i=kc+dir;i>0 && i<7;i+=dir)
            if(checkAttack(kl,i)) {
                turn = -turn;
                return false;
            }
        turn = -turn;
        return true;
    }

    bool checkCheck()
    {
        int i;
        turn = -turn;
        for(i=0;i<npeca;i++) {
            if(peca[i].type==KING && peca[i].owner==turn) {
                bool res = checkAttack(peca[i].l,peca[i].c);
                turn = -turn;
                return res;
            }
        }
        turn = -turn;
        return false;
    }

    int listMoves(_move *list)
    {
        int n=0;
        int l,c,x,y,k,kc=-1,kl=-1;

        for(k=0;k<npeca;k++) {
            if(peca[k].owner==turn) {
                l=peca[k].l;
                c=peca[k].c;
                switch(peca[k].type) {
                    case PAWN:
                        y=l+turn;
                        x=c;
                        if(b[y][x].type==CLEAR) {
                            put();
                            y+=turn;
                            if(l==(7-5*turn)/2 && b[y][x].type==CLEAR) {
                                put();
                            }
                            y-=turn;
                        }
                        x++;
                        if(x<8 && b[y][x].type!=0 && b[y][x].owner==-turn)
                            put();
                        x-=2;
                        if(x>=0 && b[y][x].type!=0 && b[y][x].owner==-turn)
                            put();
                        break;
                    case KNIGHT:
                        x=c+2;
                        y=l+1;
                        if(x<8 && y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c+2;
                        y=l-1;
                        if(x<8 && y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c+1;
                        y=l+2;
                        if(x<8 && y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c+1;
                        y=l-2;
                        if(x<8 && y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-1;
                        y=l+2;
                        if(x>=0 && y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-1;
                        y=l-2;
                        if(x>=0 && y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-2;
                        y=l+1;
                        if(x>=0 && y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-2;
                        y=l-1;
                        if(x>=0 && y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        break;
                    case QUEEN:
                        x=c;y=l;
                        for(x++;x<8 && b[y][x].type==CLEAR;x++)
                            put();
                        if(x<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x--;x>=0 && b[y][x].type==CLEAR;x--)
                            put();
                        if(x>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(y++;y<8 && b[y][x].type==CLEAR;y++)
                            put();
                        if(y<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(y--;y>=0 && b[y][x].type==CLEAR;y--)
                            put();
                        if(y>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x++,y++;x<8 && y<8 && b[y][x].type==CLEAR;x++,y++)
                            put();
                        if(x<8 && y<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x++,y--;x<8 && y>=0 && b[y][x].type==CLEAR;x++,y--)
                            put();
                        if(x<8 && y>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x--,y--;x>=0 && y>=0 && b[y][x].type==CLEAR;x--,y--)
                            put();
                        if(x>=0 && y>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x--,y++;x>=0 && y<8 && b[y][x].type==CLEAR;x--,y++)
                            put();
                        if(x>=0 && y<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        break;
                    case ROOK:
                        x=c;y=l;
                        for(x++;x<8 && b[y][x].type==CLEAR;x++)
                            put();
                        if(x<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x--;x>=0 && b[y][x].type==CLEAR;x--)
                            put();
                        if(x>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(y++;y<8 && b[y][x].type==CLEAR;y++)
                            put();
                        if(y<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(y--;y>=0 && b[y][x].type==CLEAR;y--)
                            put();
                        if(y>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        break;
                    case BISHOP:
                        x=c;y=l;
                        for(x++,y++;x<8 && y<8 && b[y][x].type==CLEAR;x++,y++)
                            put();
                        if(x<8 && y<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x++,y--;x<8 && y>=0 && b[y][x].type==CLEAR;x++,y--)
                            put();
                        if(x<8 && y>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x--,y--;x>=0 && y>=0 && b[y][x].type==CLEAR;x--,y--)
                            put();
                        if(x>=0 && y>=0 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        x=c;y=l;
                        for(x--,y++;x>=0 && y<8 && b[y][x].type==CLEAR;x--,y++)
                            put();
                        if(x>=0 && y<8 && b[y][x].type && b[y][x].owner==-turn)
                            put();
                        break;
                    case KING:
                        kc=c;kl=l;
                        x=c+1;y=l+1;
                        if(x<8 && y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c;y=l+1;
                        if(y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-1;y=l+1;
                        if(x>=0 && y<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-1;y=l;
                        if(x>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c-1;y=l-1;
                        if(x>=0 && y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c;y=l-1;
                        if(y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c+1;y=l-1;
                        if(x<8 && y>=0 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        x=c+1;y=l;
                        if(x<8 && (b[y][x].type==CLEAR || b[y][x].owner!=turn))
                            put();
                        break;
                }
            }
        }
        return n;
    }

    bool checkMate() {
        _move moves[1024];
        MoveInfo mi;
        if(not checkCheck()) {
            return false;
        }
        int n = listMoves(moves);
        int kl=0, kc=0;
        for(int i=0;i<npeca;++i) {
            if(peca[i].type == KING and peca[i].owner == turn) {
                kl = peca[i].l;
                kc = peca[i].c;
                break;
            }
        }
        bool res = true;
        for(int i=0;i<n;++i) {
            mi = move(moves[i].ol, moves[i].oc, moves[i].dl, moves[i].dc);
            if(kl == moves[i].ol and kc == moves[i].oc) {
                if(not checkAttack(moves[i].dl, moves[i].dc)) {
                    res = false;
                }
            } else {
                if(not checkAttack(kl, kc)) {
                    res = false;
                }
            }
            volta(moves[i].ol, moves[i].oc, moves[i].dl, moves[i].dc, mi);
        }
        return res;
    }
};

const int valtable[7]={0,1,5,3,3,9,1000000};

int ComputerPlay(int turn);
int Compute(int turn, int depth, bool save_best = true, int min=-0x7fffff00, int max=0x7fffff00);
int EvaluatePos();

#define dificuldade 8

Board board;
_move list[256];

int eval;
long long node_count;
_move best;
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

    srand48(seed);
    board.setInitial();
    board.checkBoard(__LINE__);

    while(1) {
        if(fgets(comando, sizeof(comando), stdin) == NULL)
            break;
        fwrite(comando,1,strlen(comando), log_file);
        comando[strlen(comando)-1]=0;
        fflush(log_file);
        int i;
        for(i = 0; comando[i]!=0 && !isspace(comando[i]);++i);
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
            board.turn = WHITE;
        } else if(strcmp(comando, "variant")==0) {
        } else if(strcmp(comando, "quit")==0) {
            break;
        } else if(strcmp(comando, "random")==0) {
            random_play=1-random_play;
        } else if(strcmp(comando, "force")==0) {
            go = 0;
        } else if(strcmp(comando, "go")==0) {
            if(go == 0) {
                play = board.turn;
                go = 1;
            }
        } else if(strcmp(comando, "playother")==0) {
            play = -board.turn;
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
            int ol,oc,dl,dc;
            if(strncmp(args,"o-o-o",5)==0) {
                if(board.turn == WHITE)
                    strcpy(args,"e1c1");
                else
                    strcpy(args,"e8c8");
            }
            if(strncmp(args,"o-o",3)==0) {
                if(board.turn == WHITE)
                    strcpy(args,"e1g1");
                else
                    strcpy(args,"e8g8");
            }
            oc=args[0]-'a';
            ol=args[1]-'1';
            dc=args[2]-'a';
            dl=args[3]-'1';
            board.move(ol, oc, dl, dc);
            if(board.checkMate()) {
                printf("result %s\n",board.turn==BLACK?"1-0":"0-1");
                go=0;
            }
            play=board.turn;
            board.print(log_file);
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
        } else if(strcmp(comando, "analyze")==0) {
        } else if(strcmp(comando, "name")==0) {
        } else if(strcmp(comando, "rating")==0) {
        } else if(strcmp(comando, "ics")==0) {
        } else if(strcmp(comando, "computer")==0) {
        } else if(strcmp(comando, "pause")==0) {
        } else if(strcmp(comando, "resume")==0) { 
        } else if(strlen(comando)==4) {
            /* play it*/
            int ol,oc,dl,dc;
            oc=comando[0]-'a';
            ol=comando[1]-'1';
            dc=comando[2]-'a';
            dl=comando[3]-'1';
            board.move(ol,oc,dl,dc);
            if(board.checkMate()) {
                printf("result %s\n",board.turn==BLACK?"1-0":"0-1");
                go=0;
            }
            play=board.turn;
            go = 1;
            board.print(log_file);
        }
        fflush(stdout);
        if(play==board.turn && go) {
            int r = ComputerPlay(board.turn);
            int ol,oc,dl,dc;
            ol=best.ol;
            oc=best.oc;
            dl=best.dl;
            dc=best.dc;
            printf("move %c%c%c%c\n",best.oc+'a',best.ol+'1',best.dc+'a',best.dl+'1');
            board.move(ol,oc,dl,dc);
            fprintf(log_file,"%d\n",r);
            if(board.checkMate()) {
                printf("%s {mate}\n", board.turn == BLACK?"1-0":"0-1");
                go=0;
            }
            board.print(log_file);
        }
        if(board.checkMate()) {
            printf("%s {mate}\n",board.turn == BLACK?"1-0":"0-1");
            go = 0;
        }
        fflush(stdout);
        fflush(log_file);
    }
    fprintf(log_file, "saindo...\n");
    return 0;
}

inline bool compara_move(const _move& m1, const _move& m2) {
    /*return (valtable[Board.b[m1.dl][m1.dc].id] > valtable[Board.b[m2.dl][m2.dc].id]) ||
      ((valtable[Board.b[m1.dl][m1.dc].id] == valtable[Board.b[m2.dl][m2.dc].id]) &&
      (valtable[Board.b[m1.ol][m1.oc].id] > valtable[Board.b[m2.ol][m2.oc].id]));*/
    return (valtable[board.b[m1.dl][m1.dc].type] > valtable[board.b[m2.dl][m2.dc].type]);
}

_move ordena_table[11][256];
int ordena_count[11];
inline void ordena_move(_move* begin, _move* end) {
    for(int i=0;i<11;++i) ordena_count[i]=0;
    for(_move* p1 = begin; p1 != end; ++p1) {
        int s = valtable[board.b[p1->dl][p1->dc].type];
        if(s >= 11) s = 10;
        ordena_table[s][ordena_count[s]++] = *p1;
    }
    for(int i=10;i>=0;--i) {
        for(int j=0;j<ordena_count[i];++j)
            *(begin++) = ordena_table[i][j];
    }
}

int Compute(int turn, int depth, bool save_best, int min, int max) {
    int n,i,t,mj;
    MoveInfo mi;
    _move list[150];
    n = board.listMoves(list);
    if(random_play && depth == dificuldade)
        random_shuffle(list, list+n);
    ordena_move(list, list+n);
    //sort(list, list+n, compara_move);
    mj=-0x7fffffff*turn;
    for(i=0;i<n;i++) {
        node_count++;
        mi = board.move(list[i].ol,list[i].oc,list[i].dl,list[i].dc);
        eval += valtable[mi.p.type]*turn;
        if(mi.pro)
            eval += turn*(valtable[QUEEN]-valtable[PAWN]);
        if(depth==1) {
            t=eval;
        } else {
            if(turn==WHITE)
                t=Compute(-turn,depth-1,false,mj,max);
            else
                t=Compute(-turn,depth-1,false,min,mj);
        }
        if(turn==WHITE) {
            if(t>mj) {
                mj=t;
                if(save_best)
                    best=list[i];
            }
        } else {
            if(t<mj) {
                mj=t;
                if(save_best)
                    best=list[i];
            }
        }

        eval -= valtable[mi.p.type]*turn;
        if(mi.pro)
            eval -= turn*(valtable[QUEEN]-valtable[PAWN]);
        board.volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc,mi);
        if(turn==WHITE) {
            if(mj>=max) return max;
        } else {
            if(mj<=min) return min;
        }
        if(depth==dificuldade) {
            fprintf(log_file,"eval %c%c%c%c %d\n", 'a'+list[i].oc,'1'+list[i].ol,'a'+list[i].dc,'1'+list[i].dl,t);
            fflush(log_file);
        }
        //CheckBoard(__LINE__);
    }
    return mj;
}

int ComputerPlay(int turn)
{
    int r;
    node_count=0;
    eval=EvaluatePos();
    if(turn==WHITE)
        r = Compute(WHITE, dificuldade);
    else
        r = Compute(BLACK, dificuldade);
    fprintf(log_file,"%lld nodes\n",node_count);
    return r;
}

int EvaluatePos()
{
    int i,v;
    for(i=0,v=0;i<board.npeca;i++) {
        v+=valtable[(int)board.peca[i].type]*(board.peca[i].owner);
    }
    return v;
}
