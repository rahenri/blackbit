#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

#define clear 0
#define pawn 1
#define rook 2
#define knight 3
#define bishop 4
#define queen 5
#define king 6
#define white 1  
#define black -1 
#define true 1
#define false 0
#define castle 2
#define vl(x) ((x)<0 && (x)>7)
#define put() {list[n].ol=l;list[n].oc=c;list[n].dl=y;list[n].dc=x;n++;}

#define movex(ol,oc,dl,dc) {\
    Board.b[dl][dc]=Board.b[ol][oc];\
    Board.b[ol][oc].type=clear;\
    peca[Board.b[dl][dc].id].l=dl;\
    peca[Board.b[dl][dc].id].c=dc;\
}


typedef struct _move
{
    int ol,oc,dl,dc;
}_move;

typedef struct _peca
{
    int l,c;
    int type;
    int owner;
}_peca;

typedef struct _pos
{
    int id;
    int type;
    int owner;
}_pos;

typedef struct _board
{
    int passan;
    char ca[3][2];
    _pos b[8][8];
}_board;

const int tabela[7]={' ','P','T','C','B','D','R'};
const int tabela2[3]={'P',' ','B'};
const int valtable[7]={0,100,500,300,300,900,100000};

void ClearBoard();
void SetBoard();
void PrintBoard(FILE* f);
bool ValidateMove(int,int,int,int,int);
void MakeMove(int,int,int,int,int);
bool ValidateMoveEx(char *,int *,int,int,int,int,int);
bool CheckAttack(int l,int c,int turn);
bool CheckCheck(int turn);
bool ValidateCastle(int,int);
int ListMoves(_move *,int);
int ComputerPlay(int turn);
int Compute(int turn, int depth, bool save_best = true, int min=-0x7fffff00, int max=0x7fffff00);
int EvaluatePos();
void CheckBoard(int line = 0);
void read_fen(char* args);
