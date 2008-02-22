#include <algorithm>
#include <ctype.h>
#include <sys/time.h>
#include "main.hh"

using namespace std;

#define dificuldade 6

_board Board;
_move list[256];
_peca peca[32];
int npeca;
int turn;
int eval;
FILE* log_file;
long long node_count;
_move best;
int random_play;

int main(int argc, char** argv)
{
    char comando[1024];
    char* args;
    int play=black;
    turn=1;
    if(argc==1)
        log_file = fopen("tmp.txt","a");
    else
        log_file = fopen(argv[1],"a");

    struct timeval time;
    gettimeofday(&time, 0);
    int seed = time.tv_usec;
    fprintf(log_file,"seed %d\n", seed);
    srand(time.tv_usec);
    srand48(time.tv_usec);
    SetBoard();
    int go = 0;
    random_play=0;

    while (1) {
        while(1) {
            if(fgets(comando, sizeof(comando), stdin) == NULL)
                return 0;
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
                SetBoard();
                play = black;
                go = 1;
                turn = white;
            } else if(strcmp(comando, "variant")==0) {
            } else if(strcmp(comando, "quit")==0) {
                return 0;
            } else if(strcmp(comando, "random")==0) {
                random_play=1-random_play;
            } else if(strcmp(comando, "force")==0) {
                go = 0;
            } else if(strcmp(comando, "go")==0) {
                if(go == 0) {
                    play = turn;
                    go = 1;
                }
            } else if(strcmp(comando, "playother")==0) {
                play=-turn;
                go=1;
            } else if(strcmp(comando, "white")==0) {
            } else if(strcmp(comando, "black")==0) {
            } else if(strcmp(comando, "level")==0) {
            } else if(strcmp(comando, "st")==0) {
            } else if(strcmp(comando, "sd")==0) {
            } else if(strcmp(comando, "time")==0) {
            } else if(strcmp(comando, "otim")==0) {
            } else if(strcmp(comando, "board")==0) {
                PrintBoard(stdout);
            } else if(strcmp(comando, "ping")==0) {
                printf("pong %s\n", args);
            } else if(strcmp(comando, "usermove")==0) {
                /* play it*/
                int ol,oc,dl,dc;
                if(strncmp(args,"o-o-o",5)==0) {
                    if(turn==white)
                        strcpy(args,"e1c1");
                    else
                        strcpy(args,"e8c8");
                }
                if(strncmp(args,"o-o",3)==0) {
                    if(turn==white)
                        strcpy(args,"e1g1");
                    else
                        strcpy(args,"e8g8");
                }
                oc=args[0]-'a';
                ol=args[1]-'1';
                dc=args[2]-'a';
                dl=args[3]-'1';
                MakeMove(ol,oc,dl,dc,turn);
                turn=-turn;
                if(CheckCheck(turn) && ListMoves(list,turn)==0) {
                    printf("result %s\n",turn==-1?"1-0":"0-1");
                    go=0;
                }
                play=turn;
                PrintBoard(log_file);
            } else if(strcmp(comando, "draw")==0) {
            } else if(strcmp(comando, "result")==0) {
                go=0;
            } else if(strcmp(comando, "setboard")==0) {
                read_fen(args);
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
                MakeMove(ol,oc,dl,dc,turn);
                turn=-turn;
                if(CheckCheck(turn) && ListMoves(list,turn)==0) {
                    printf("result %s\n",turn==-1?"1-0":"0-1");
                    go=0;
                }
                play=turn;
                go = 1;
                PrintBoard(log_file);
            }
            fflush(stdout);
            if(play==turn && go) {
                int r = ComputerPlay(turn);
                int ol,oc,dl,dc;
                ol=best.ol;
                oc=best.oc;
                dl=best.dl;
                dc=best.dc;
                printf("move %c%c%c%c\n",best.oc+'a',best.ol+'1',best.dc+'a',best.dl+'1');
                MakeMove(ol,oc,dl,dc,turn);
                fprintf(log_file,"%d\n",r);
                if(CheckCheck(turn) && ListMoves(list,turn)==0) {
                    printf("%s {mate}\n",turn==-1?"1-0":"0-1");
                    go=0;
                }
                turn=-turn;
                PrintBoard(log_file);
            }
            if(CheckCheck(turn) && ListMoves(list,turn)==0) {
                printf("%s {mate}\n",turn==-1?"1-0":"0-1");
                go = 0;
            }
            fflush(stdout);
            fflush(log_file);
        }
    }
    fprintf(log_file, "saindo...\n");
    return 0;
}

void ClearBoard()
{
    memset(Board.b,0,sizeof(Board.b));
    memset(Board.ca,1,sizeof(Board.ca));
    Board.passan = -1;
}

void read_fen(char* args) {
    int linha=7, coluna=0, i;
    ClearBoard();
    for(i=0;args[i]!=' ';++i) {
        if(args[i]=='/') 
            --linha, coluna=0;
        else if(args[i]>='1' && args[i]<='8')
            coluna+=args[i]-'0';
        else {
            Board.b[linha][coluna].owner = isupper(args[i])?white:black;
            switch(tolower(args[i])) {
                case 'r': Board.b[linha][coluna].type = rook; break;
                case 'n': Board.b[linha][coluna].type = knight; break;
                case 'b': Board.b[linha][coluna].type = bishop; break;
                case 'q': Board.b[linha][coluna].type = queen; break;
                case 'k': Board.b[linha][coluna].type = king; break;
                case 'p': Board.b[linha][coluna].type = pawn; break;
            }
            ++coluna;
        }
    }
    ++i;
    if(tolower(args[i])=='w')
        turn = white;
    else
        turn = black;
    npeca=0;
    for(linha=0;linha<8;linha++) {
        for(coluna=0;coluna<8;coluna++) {
            if(Board.b[linha][coluna].type) {
                peca[npeca].l=linha;
                peca[npeca].c=coluna;
                peca[npeca].owner=Board.b[linha][coluna].owner;
                peca[npeca].type=Board.b[linha][coluna].type;
                Board.b[linha][coluna].id=npeca;
                npeca++;
            }
        }
    }
}

void SetBoard()
{
    int i,l,c;
    ClearBoard();
    Board.b[0][0].type=Board.b[0][7].type=rook;
    Board.b[0][1].type=Board.b[0][6].type=knight;
    Board.b[0][2].type=Board.b[0][5].type=bishop;
    Board.b[0][3].type=queen;
    Board.b[0][4].type=king;
    Board.b[0][0].owner=Board.b[0][7].owner=white;
    Board.b[0][1].owner=Board.b[0][6].owner=white;
    Board.b[0][2].owner=Board.b[0][5].owner=white;
    Board.b[0][3].owner=white;
    Board.b[0][4].owner=white;
    for(i=0;i<8;i++)
    {
        Board.b[1][i].type=pawn;
        Board.b[1][i].owner=white;
    }
    Board.b[7][0].type=Board.b[7][7].type=rook;
    Board.b[7][1].type=Board.b[7][6].type=knight;
    Board.b[7][2].type=Board.b[7][5].type=bishop;
    Board.b[7][3].type=queen;
    Board.b[7][4].type=king;
    Board.b[7][0].owner=Board.b[7][7].owner=black;
    Board.b[7][1].owner=Board.b[7][6].owner=black;
    Board.b[7][2].owner=Board.b[7][5].owner=black;
    Board.b[7][3].owner=black;
    Board.b[7][4].owner=black;
    for(i=0;i<8;i++)
    {
        Board.b[6][i].type=pawn;
        Board.b[6][i].owner=black;
    }
    npeca=0;
    for(l=0;l<8;l++)
    {
        for(c=0;c<8;c++)
        {
            if(Board.b[l][c].type)
            {
                peca[npeca].l=l;
                peca[npeca].c=c;
                peca[npeca].owner=Board.b[l][c].owner;
                peca[npeca].type=Board.b[l][c].type;
                Board.b[l][c].id=npeca;
                npeca++;
            }
        }
    }
}

void PrintBoard(FILE * f)
{
    int i,l;
    unsigned char linha[128];
    linha[0]='.';
    linha[1]='-';
    linha[2]='-';
    for(i=1;i<8;i++)
    {
        linha[3*i]='-';
        linha[3*i+1]='-';
        linha[3*i+2]='-';
    }
    linha[3*i]='.';
    linha[3*i+1]=0;
    fprintf(f,"%s\n",linha);
    for(l=7;;l--)
    {
        for(i=0;i<8;i++)
        {
            linha[3*i]='|';
            if(Board.b[l][i].type==clear)
            {
                linha[3*i+1]=linha[3*i+2]=' ';
                continue;
            }
            linha[3*i+1]=tabela[(int)Board.b[l][i].type];
            linha[3*i+2]=tabela2[Board.b[l][i].owner+1];
        }
        linha[3*i]='|';
        linha[3*i+1]=0;
        fprintf(f,"%s\n",linha);
        linha[0]='|';
        linha[1]='-';
        linha[2]='-';
        if(l==0)
            break;
        for(i=1;i<8;i++)
        {
            linha[3*i]='+';
            linha[3*i+1]=linha[3*i+2]='-';
        }
        linha[3*i]='|';
        linha[3*i+1]=0;
        fprintf(f,"%s\n",linha);
    }
    linha[0]='*';
    linha[1]=linha[2]='-';
    for(i=1;i<8;i++)
    {
        linha[3*i]='-';
        linha[3*i+1]=linha[3*i+2]='-';
    }
    linha[3*i]='*';
    linha[3*i+1]=0;
    fprintf(f,"%s\n",linha);
}


bool ValidateMove(int ol,int oc,int dl,int dc,int turn) {
    int i,j,di,dj;
    if(Board.b[ol][oc].type==clear || (Board.b[ol][oc].owner)!=turn) {
        return false;
    }
    switch(Board.b[ol][oc].type) {
        case pawn:
            if(Board.b[dl][dc].type==clear) {
                if(dc!=oc)
                    return false;
                if((dl-ol)*turn>2 || (dl-ol)*turn<=0)
                    return false;
                if(ol!=((7-turn*5)/2) && (dl-ol)*turn==2)
                    return false;
                if(ol==(7+turn*5)/2 && (dl-ol)*turn==2 && Board.b[ol+dl/2][oc].type!=clear)
                    return false;
            } else if((oc-dc)==1 || (oc-dc)==-1) {
                if(abs(dl-ol)!=1)
                    return false;
            }
            else
                return false;
            break;
        case knight:
            if(abs((ol-dl)*(oc-dc))==2 &&( (Board.b[dl][dc].owner)==-turn || Board.b[dl][dc].type==clear ))
                return true;
            else
                return false;
        case bishop:
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
            for(i=oc+di,j=ol+dj;i!=dc;i+=di, j+=dj)
                if(Board.b[j][i].type)
                    return false;
            if(Board.b[dl][dc].owner==turn && Board.b[dl][dc].type!=clear)
                return false;
            break;
        case king:
            if(abs(oc-dc)==2 && dl==ol) {
                if(dc>oc)
                    i=1;
                else
                    i=-1;
                if(ValidateCastle(i,turn))
                    return castle;
                else
                    return false;
            }
            if((abs(ol-dl)|abs(oc-dc))!=1)
                return false;
            if(Board.b[dl][dc].owner==turn && Board.b[dl][dc].type!=clear)
                return false;
            break;
        case rook:
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
            for(i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
                if(Board.b[j][i].type)
                    return false;
            if(Board.b[dl][dc].type!=clear && Board.b[dl][dc].owner==turn)
                return false;
            break;
        case queen:
            if(!((oc!=dc) ^ (ol!=dl)) && (abs(oc-dc)!=abs(ol-dl)))
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
            for(i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
                if(Board.b[j][i].type)
                    return false;
            if(Board.b[dl][dc].owner==turn && Board.b[dl][dc].type!=clear)
                return false;
            break;
    }
    return true;
}

bool CheckAttack(int l,int c,int turn)
{
    int i;
    for(i=0;i<npeca;i++) {
        if(peca[i].owner==-turn)
            if(ValidateMove(peca[i].l,peca[i].c,l,c,-turn))
                return true;
    }
    return false;
}

bool CheckCheck(int turn)
{
    int i;
    for(i=0;i<npeca;i++)
        if(peca[i].type==king && peca[i].owner==turn)
            return CheckAttack(peca[i].l,peca[i].c,turn);
    return false;
}

bool ValidateCastle(int dir, int turn)
{
    int i,kl,kc;
    kl=(7-turn*7)/2;
    kc=4;
    if(Board.b[kl][kc].type!=king || Board.b[kl][kc].owner!=turn)
        return false;
    if(Board.b[kl][(7+7*dir)/2].type!=rook || Board.b[kl][(7+7*dir)/2].owner!=turn)
        return false;
    for(i=kc+dir;i>0 && i<7;i+=dir)
        if(Board.b[kl][i].type)
            return false;
    if(CheckAttack(kl,kc,turn))
        return false;
    for(i=kc+dir;i>0 && i<7;i+=dir)
        if(CheckAttack(kl,i,turn))
            return false;
    return true;
}

void move(int ol,int oc,int dl,int dc,int turn, int& wp,_pos& b) {
    b=Board.b[dl][dc];
    Board.b[dl][dc]=Board.b[ol][oc];
    Board.b[ol][oc].type=clear;
    peca[Board.b[dl][dc].id].l=dl;
    peca[Board.b[dl][dc].id].c=dc;
    if(b.type!=clear) {
        if(b.id!=npeca-1) {
            peca[b.id] = peca[npeca-1];
            Board.b[peca[b.id].l][peca[b.id].c].id = b.id;
        }
        npeca--;
    }
    if(Board.b[dl][dc].type == pawn && (dl==7 || dl==0)) {
        Board.b[dl][dc].type = queen;
        peca[Board.b[dl][dc].id].type = queen;
        wp=1;
    } else {
        wp=0;
    }
}

void volta(int ol,int oc,int dl,int dc,int turn,int wp,_pos b) {
    Board.b[ol][oc]=Board.b[dl][dc];
    Board.b[dl][dc]=b;
    peca[Board.b[ol][oc].id].l=ol;
    peca[Board.b[ol][oc].id].c=oc;
    if(b.type != clear) {
        peca[npeca].type=b.type;
        peca[npeca].owner=b.owner;
        peca[npeca].l=dl;
        peca[npeca].c=dc;
        Board.b[dl][dc].id=npeca;
        npeca++;
    }
    if(wp) {
        Board.b[ol][oc].type=pawn;
        peca[Board.b[ol][oc].id].type=pawn;
    }
}


bool compara_move(const _move& m1, const _move& m2) {
    /*return (valtable[Board.b[m1.dl][m1.dc].id] > valtable[Board.b[m2.dl][m2.dc].id]) ||
      ((valtable[Board.b[m1.dl][m1.dc].id] == valtable[Board.b[m2.dl][m2.dc].id]) &&
      (valtable[Board.b[m1.ol][m1.oc].id] > valtable[Board.b[m2.ol][m2.oc].id]));*/
    return (valtable[Board.b[m1.dl][m1.dc].type] > valtable[Board.b[m2.dl][m2.dc].type]);
}

int ListMoves(_move *list,int turn)
{
    int n=0;
    int l,c,x,y,i,j,k,wp,kc=-1,kl=-1;
    _pos b;

    for(k=0;k<npeca;k++) {
        if(peca[k].owner==turn) {
            l=peca[k].l;
            c=peca[k].c;
            switch(peca[k].type) {
                case pawn:
                    y=l+turn;
                    x=c;
                    if(Board.b[y][x].type==clear) {
                        put();
                        y+=turn;
                        if(l==(7-5*turn)/2 && Board.b[y][x].type==clear) {
                            put();
                        }
                        y-=turn;
                    }
                    x++;
                    if(x<8 && Board.b[y][x].type!=0 && Board.b[y][x].owner==-turn)
                        put();
                    x-=2;
                    if(x>=0 && Board.b[y][x].type!=0 && Board.b[y][x].owner==-turn)
                        put();
                    break;
                case knight:
                    x=c+2;
                    y=l+1;
                    if(x<8 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c+2;
                    y=l-1;
                    if(x<8 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c+1;
                    y=l+2;
                    if(x<8 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c+1;
                    y=l-2;
                    if(x<8 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-1;
                    y=l+2;
                    if(x>=0 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-1;
                    y=l-2;
                    if(x>=0 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-2;
                    y=l+1;
                    if(x>=0 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-2;
                    y=l-1;
                    if(x>=0 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    break;
                case queen:
                    x=c;y=l;
                    for(x++;x<8 && Board.b[y][x].type==clear;x++)
                        put();
                    if(x<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x--;x>=0 && Board.b[y][x].type==clear;x--)
                        put();
                    if(x>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(y++;y<8 && Board.b[y][x].type==clear;y++)
                        put();
                    if(y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(y--;y>=0 && Board.b[y][x].type==clear;y--)
                        put();
                    if(y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x++,y++;x<8 && y<8 && Board.b[y][x].type==clear;x++,y++)
                        put();
                    if(x<8 && y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x++,y--;x<8 && y>=0 && Board.b[y][x].type==clear;x++,y--)
                        put();
                    if(x<8 && y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x--,y--;x>=0 && y>=0 && Board.b[y][x].type==clear;x--,y--)
                        put();
                    if(x>=0 && y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x--,y++;x>=0 && y<8 && Board.b[y][x].type==clear;x--,y++)
                        put();
                    if(x>=0 && y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    break;
                case rook:
                    x=c;y=l;
                    for(x++;x<8 && Board.b[y][x].type==clear;x++)
                        put();
                    if(x<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x--;x>=0 && Board.b[y][x].type==clear;x--)
                        put();
                    if(x>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(y++;y<8 && Board.b[y][x].type==clear;y++)
                        put();
                    if(y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(y--;y>=0 && Board.b[y][x].type==clear;y--)
                        put();
                    if(y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    break;
                case bishop:
                    x=c;y=l;
                    for(x++,y++;x<8 && y<8 && Board.b[y][x].type==clear;x++,y++)
                        put();
                    if(x<8 && y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x++,y--;x<8 && y>=0 && Board.b[y][x].type==clear;x++,y--)
                        put();
                    if(x<8 && y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x--,y--;x>=0 && y>=0 && Board.b[y][x].type==clear;x--,y--)
                        put();
                    if(x>=0 && y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    x=c;y=l;
                    for(x--,y++;x>=0 && y<8 && Board.b[y][x].type==clear;x--,y++)
                        put();
                    if(x>=0 && y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
                        put();
                    break;
                case king:
                    kc=c;kl=l;
                    x=c+1;y=l+1;
                    if(x<8 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c;y=l+1;
                    if(y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-1;y=l+1;
                    if(x>=0 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-1;y=l;
                    if(x>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c-1;y=l-1;
                    if(x>=0 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c;y=l-1;
                    if(y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c+1;y=l-1;
                    if(x<8 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    x=c+1;y=l;
                    if(x<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
                        put();
                    break;
            }
        }
    }
    for(i=0,j=0;i<n;i++) {
        // movimento com o rei?
        if(kc==list[i].oc&&kl==list[i].ol) {
            move(list[i].ol,list[i].oc,list[i].dl,list[i].dc,turn,wp,b);
            if(!CheckAttack(list[i].dl,list[i].dc,turn)) {
                list[j]=list[i];
                j++;
            }
            volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc,turn,wp,b);
        } else {
            move(list[i].ol,list[i].oc,list[i].dl,list[i].dc,turn,wp,b);
            if(!CheckAttack(kl,kc,turn)) {
                list[j]=list[i];
                j++;
            }
            volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc,turn,wp,b);
        }
    }
    n=j;
    return n;
}

int Compute(int turn, int depth, bool save_best, int min, int max) {
    int n,i,t,mj,wp;
    _pos b;
    _move list[150];
    n=ListMoves(list,turn);
    if(random_play && depth == dificuldade)
        random_shuffle(list, list+n);
    stable_sort(list,list+n,compara_move);
    if(n==0) {
        if(CheckCheck(turn)) {
            return -0x7fffff00*turn;
        } else {
            return 0;
        }
    }
    mj=-0x7fffffff*turn;
    for(i=0;i<n;i++) {
        node_count++;
        move(list[i].ol,list[i].oc,list[i].dl,list[i].dc,turn,wp,b);
        eval+=valtable[b.type]*turn;
        if(wp)
            eval+=turn*(valtable[queen]-valtable[pawn]);
        if(depth==1) {
            t=eval;
        } else {
            if(turn==white)
                t=Compute(-turn,depth-1,false,mj,max);
            else
                t=Compute(-turn,depth-1,false,min,mj);
        }
        if(turn==white) {
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

        eval-=valtable[b.type]*turn;
        if(wp)
            eval -= turn*(valtable[queen]-valtable[pawn]);
        volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc,turn,wp,b);
        if(turn==white) {
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
    if(turn==white)
        r = Compute(white, dificuldade);
    else
        r = Compute(black, dificuldade);
    fprintf(log_file,"%lld nodes\n",node_count);
    return r;
}

int EvaluatePos()
{
    int i,v;
    for(i=0,v=0;i<npeca;i++) {
        v+=valtable[(int)peca[i].type]*(peca[i].owner);
    }
    return v;
}

void CheckBoard(int line)
{
    int i,j,n;
    for(i=0,n=0;i<8;i++) {
        for(j=0;j<8;j++) {
            if(Board.b[i][j].type)
                n++;
        }
    }
    if(n!=npeca) {
        fprintf(log_file,"%d :Contagem no bate\n", line);
        return;
    }
    for(i=0;i<npeca;i++) {
        if(Board.b[peca[i].l][peca[i].c].type!=peca[i].type || Board.b[peca[i].l][peca[i].c].owner!=peca[i].owner || Board.b[peca[i].l][peca[i].c].id!=i) {
            fprintf(log_file,"%d: (%d,%d) -> peca(type=%d,owner=%d,id=%d) Board(type=%d,owner=%d,id=%d)\n",line,peca[i].l,peca[i].c,peca[i].type,peca[i].owner,i,Board.b[peca[i].l][peca[i].c].type,Board.b[peca[i].l][peca[i].c].owner,Board.b[peca[i].l][peca[i].c].id);
            PrintBoard(log_file);
            exit(1);
            return;
        }
    }
}

void MakeMove(int ol,int oc,int dl,int dc,int turn) {
    fprintf(log_file,"%c%c%c%c\n",oc+'a',ol+'1',dc+'a',dl+'1');
    CheckBoard(__LINE__);
    _pos b;
    b=Board.b[dl][dc];
    Board.b[dl][dc]=Board.b[ol][oc];
    Board.b[ol][oc].type=clear;
    peca[Board.b[dl][dc].id].l=dl;
    peca[Board.b[dl][dc].id].c=dc;
    if(b.type!=clear) {
        if(b.id!=npeca-1) {
            peca[b.id] = peca[npeca-1];
            Board.b[peca[b.id].l][peca[b.id].c].id = b.id;
        }
        npeca--;
    }
    if(Board.b[dl][dc].type==pawn && dc != oc && b.type==clear) {
        b=Board.b[ol][dl];
        Board.b[ol][dl].type=clear;
        if(b.type!=clear) {
            if(b.id!=npeca-1) {
                peca[b.id] = peca[npeca-1];
                Board.b[peca[b.id].l][peca[b.id].c].id = b.id;
            }
            npeca--;
        }
    }
    if(Board.b[dl][dc].type == pawn && (dl==7 || dl==0)) {
        Board.b[dl][dc].type = queen;
        peca[Board.b[dl][dc].id].type = queen;
    }
    if(Board.b[dl][dc].type == king && (dc-oc)==2) {
        movex(dl,7,dl,5);
    }
    if(Board.b[dl][dc].type == king && (dc-oc)==-2) {
        movex(dl,0,dl,3);
    }
    CheckBoard(__LINE__);
}

